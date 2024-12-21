// src/MaskingASTVisitor.h
#ifndef MASKING_AST_VISITOR_H
#define MASKING_AST_VISITOR_H

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "SensitiveVariableFinder.h"
#include <string>
#include <set>
#include <unordered_map>
#include <clang/AST/ASTConsumer.h>

class MaskingASTVisitor : public clang::RecursiveASTVisitor<MaskingASTVisitor> {
public:
    MaskingASTVisitor(clang::ASTContext &Context, clang::Rewriter &R);

    bool VisitFunctionDecl(clang::FunctionDecl *F);
    bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
    bool VisitBinaryOperator(clang::BinaryOperator *Op);

private:
    clang::ASTContext &Context;
    clang::Rewriter &TheRewriter;

    bool InCryptoFunction = false;
    int TempCounter = 0;

    // For each key param, we store "k_m"
    std::unordered_map<clang::VarDecl*, std::string> MaskedParamMap;
    std::set<clang::VarDecl*> SensitiveVars;

    SensitiveVariableFinder VarFinder;

    void splitKeyParams(clang::FunctionDecl *F);

    bool isSensitive(clang::Expr *E);
    bool isSensitiveVar(clang::VarDecl *VD);

    // Gadgets
    void maskAND(clang::BinaryOperator *Op);
    void maskOR(clang::BinaryOperator *Op);
    void maskXOR(clang::BinaryOperator *Op);
    void maskADD(clang::BinaryOperator *Op);
    void maskShiftLeft(clang::BinaryOperator *Op);

    // ephemeral "Masked32 tmpM = mask32_and(...);" inserted before statement
    // then expression => "(int)mask32_demask(tmpM)"
    std::string createEphemeralMaskCall(const std::string &callExpr,
                                        clang::BinaryOperator *Op);
};

class MaskingASTConsumer : public clang::ASTConsumer {
public:
    explicit MaskingASTConsumer(clang::Rewriter &R);
    void HandleTranslationUnit(clang::ASTContext &Context) override;
private:
    clang::Rewriter &TheRewriter;
    MaskingASTVisitor *Visitor;
};

#endif
