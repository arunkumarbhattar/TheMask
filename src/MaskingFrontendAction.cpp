// src/MaskingFrontendAction.cpp
#include "MaskingFrontendAction.h"
#include "MaskingASTVisitor.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"

using namespace clang;

void MaskingFrontendAction::EndSourceFileAction() {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());
}

std::unique_ptr<ASTConsumer> MaskingFrontendAction::CreateASTConsumer(
    CompilerInstance &CI, llvm::StringRef file) {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MaskingASTConsumer>(TheRewriter);
}
