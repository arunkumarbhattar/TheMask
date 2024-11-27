// src/MaskingASTVisitor.cpp
#include "MaskingASTVisitor.h"

using namespace clang;

MaskingASTVisitor::MaskingASTVisitor(ASTContext &Context, Rewriter &R)
    : Context(Context), TheRewriter(R), RandomVarCounter(0), VarFinder(), Masker() {}

bool MaskingASTVisitor::VisitFunctionDecl(FunctionDecl *F) {
    if (F->hasBody()) {
        VarFinder.TraverseDecl(F);
        SensitiveVars = VarFinder.getSensitiveVars();
    }
    return true;
}

bool MaskingASTVisitor::VisitBinaryOperator(BinaryOperator *Op) {
    Expr *LHS = Op->getLHS()->IgnoreParenImpCasts();
    Expr *RHS = Op->getRHS()->IgnoreParenImpCasts();

    bool LHSIsSensitive = isSensitive(LHS);
    bool RHSIsSensitive = isSensitive(RHS);

    if (LHSIsSensitive || RHSIsSensitive) {
        if (Op->isAdditiveOp() || Op->getOpcode() == BO_Xor) {
            maskLinearOperation(Op);
        } else {
            maskNonlinearOperation(Op);
        }
    }
    return true;
}

bool MaskingASTVisitor::isSensitive(Expr *E) {
    if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
        VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());
        return SensitiveVars.count(VD) > 0;
    }
    return false;
}

void MaskingASTVisitor::maskLinearOperation(clang::BinaryOperator *Op) {
    // Apply linear masking
    // Generate random variable
    std::string RandomVarName = "r" + std::to_string(RandomVarCounter++);
    std::string RandomVarDecl = "int " + RandomVarName + " = get_random();\n";

    // Insert random variable declaration
    TheRewriter.InsertTextBefore(Op->getExprLoc(), RandomVarDecl);

    // Replace operands with masked versions
    std::string LHSCode = Op->getLHS()->getSourceRange().printToString(TheRewriter.getSourceMgr());
    std::string RHSCode = Op->getRHS()->getSourceRange().printToString(TheRewriter.getSourceMgr());

    if (isSensitive(Op->getLHS())) {
        std::string MaskedLHS = "(" + LHSCode + " ^ " + RandomVarName + ")";
        TheRewriter.ReplaceText(Op->getLHS()->getSourceRange(), MaskedLHS);
    }

    if (isSensitive(Op->getRHS())) {
        std::string MaskedRHS = "(" + RHSCode + " ^ " + RandomVarName + ")";
        TheRewriter.ReplaceText(Op->getRHS()->getSourceRange(), MaskedRHS);
    }

    // Adjust operation if necessary
    // For linear operations, the operation remains the same
}

void MaskingASTVisitor::maskNonlinearOperation(BinaryOperator *Op) {
    std::string MaskedCode = Masker.maskNonlinearOperation(Op, RandomVarCounter);
    TheRewriter.ReplaceText(Op->getSourceRange(), MaskedCode);
}

MaskingASTConsumer::MaskingASTConsumer(Rewriter &R)
    : TheRewriter(R), Visitor(nullptr) {}

void MaskingASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    Visitor = new MaskingASTVisitor(Context, TheRewriter);
    Visitor->TraverseDecl(Context.getTranslationUnitDecl());
    delete Visitor;
}
