#ifndef MASKINGASTVISITOR_H
#define MASKINGASTVISITOR_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "SensitiveVariableFinder.h"
#include "SMTMasker.h"
#include "clang/Tooling/Tooling.h"

#include <set>

class MaskingASTVisitor : public clang::RecursiveASTVisitor<MaskingASTVisitor> {
public:
    MaskingASTVisitor(clang::ASTContext &Context, clang::Rewriter &R);

    bool VisitFunctionDecl(clang::FunctionDecl *F);
    bool VisitBinaryOperator(clang::BinaryOperator *Op);

private:
    bool InCryptoFunction = false;
    clang::ASTContext &Context;
    clang::Rewriter &TheRewriter;
    std::set<clang::VarDecl *> SensitiveVars;
    SensitiveVariableFinder VarFinder;
    SMTMasker Masker;
    int RandomVarCounter;

    bool isSensitive(clang::Expr *Expr);
    void maskLinearOperation(clang::BinaryOperator *Op);
    void maskNonlinearOperation(clang::BinaryOperator *Op);
};

class MaskingASTConsumer : public clang::ASTConsumer {
public:
    MaskingASTConsumer(clang::Rewriter &R);

    void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
    MaskingASTVisitor *Visitor;
    clang::Rewriter &TheRewriter;
};

#endif // MASKINGASTVISITOR_H
