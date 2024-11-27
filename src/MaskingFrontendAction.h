// src/MaskingFrontendAction.h
#ifndef MASKINGFRONTENDACTION_H
#define MASKINGFRONTENDACTION_H

#include "MaskingASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>

class MaskingFrontendAction : public clang::ASTFrontendAction {
public:
    void EndSourceFileAction() override;
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance &CI, llvm::StringRef file) override;

private:
    clang::Rewriter TheRewriter;
};

#endif // MASKINGFRONTENDACTION_H
