// src/MaskingFrontendAction.cpp
#include "MaskingFrontendAction.h"
#include "MaskingASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

void MaskingFrontendAction::EndSourceFileAction() {
    FileID mainFileID = TheRewriter.getSourceMgr().getMainFileID();
    // Insert masked code once
    if (!InsertedHeader) {
        SourceLocation top =
            TheRewriter.getSourceMgr().getLocForStartOfFile(mainFileID);

        // Production-level approach for 32-bit masked ops
        std::string injection =
R"CODE(#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// 32-bit random generator (placeholder).
static inline uint32_t get_random32(void) {
    // Real usage: cryptographically secure RNG.
    uint32_t r = ((uint32_t)rand() << 1) ^ (uint32_t)(rand() & 1);
    return r;
}

// "Masked32" => two 32-bit shares
typedef struct {
    uint32_t s1;
    uint32_t s2;
} Masked32;

// Create from unmasked int
static inline Masked32 mask32_create(uint32_t x) {
    Masked32 m;
    m.s2 = get_random32();
    m.s1 = x ^ m.s2;
    return m;
}

// Demask => x = s1 ^ s2
static inline uint32_t mask32_demask(Masked32 m) {
    return (m.s1 ^ m.s2);
}

// Gadgets
static inline Masked32 mask32_xor(Masked32 x, Masked32 y) {
    Masked32 out;
    out.s1 = x.s1 ^ y.s1;
    out.s2 = x.s2 ^ y.s2;
    return out;
}

static inline Masked32 mask32_and(Masked32 x, Masked32 y) {
    uint32_t r = get_random32();
    uint32_t t1 = x.s1 & y.s1;
    Masked32 out;
    out.s1 = t1 ^ r;
    out.s2 = r ^ (x.s1 & y.s2) ^ (x.s2 & y.s1) ^ (x.s2 & y.s2);
    return out;
}

static inline Masked32 mask32_or(Masked32 x, Masked32 y) {
    uint32_t r = get_random32();
    uint32_t t1 = x.s1 | y.s1;
    Masked32 out;
    out.s1 = t1 ^ r;
    out.s2 = r ^ (x.s1 | y.s2) ^ (x.s2 | y.s1) ^ (x.s2 | y.s2);
    return out;
}

static inline Masked32 mask32_add(Masked32 x, Masked32 y) {
    // naive ignoring carry leak
    uint32_t r = get_random32();
    Masked32 out;
    out.s1 = (x.s1 + y.s1) ^ r;
    out.s2 = (x.s2 + y.s2) ^ r;
    return out;
}

static inline Masked32 mask32_shiftLeft(Masked32 x, uint32_t shift) {
    Masked32 out;
    out.s1 = x.s1 << shift;
    out.s2 = x.s2 << shift;
    return out;
}
)CODE";

        TheRewriter.InsertTextBefore(top, injection);
        InsertedHeader = true;
    }

    // Write final code to stdout
    TheRewriter.getEditBuffer(mainFileID).write(llvm::outs());

    // Also write to <filename>_masked.c
    auto &SM = TheRewriter.getSourceMgr();
    std::string origFilename =
        SM.getFilename(SM.getLocForStartOfFile(mainFileID)).str();
    if (!origFilename.empty()) {
        size_t dotPos = origFilename.rfind('.');
        std::string base = (dotPos == std::string::npos)
                               ? origFilename
                               : origFilename.substr(0, dotPos);
        std::string maskedFile = base + "_masked.c";
        llvm::errs() << "Writing masked output to: " << maskedFile << "\n";

        std::error_code EC;
        llvm::raw_fd_ostream outFile(maskedFile, EC, llvm::sys::fs::OF_Text);
        if (!EC) {
            TheRewriter.getEditBuffer(mainFileID).write(outFile);
            outFile.close();
        } else {
            llvm::errs() << "Could not open " << maskedFile << " for writing!\n";
        }
    }
}

std::unique_ptr<ASTConsumer>
MaskingFrontendAction::CreateASTConsumer(CompilerInstance &CI,
                                         llvm::StringRef file) {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MaskingASTConsumer>(TheRewriter);
}
