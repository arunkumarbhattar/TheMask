// src/MaskingFrontendAction.h
#ifndef MASKING_FRONTEND_ACTION_H
#define MASKING_FRONTEND_ACTION_H

#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"

class MaskingFrontendAction : public clang::ASTFrontendAction {
public:
    void EndSourceFileAction() override;

    std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI,
                      llvm::StringRef file) override;

private:
    clang::Rewriter TheRewriter;
    bool InsertedHeader = false;
};

#endif
