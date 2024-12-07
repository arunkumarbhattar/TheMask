#include "MaskingASTVisitor.h"
#include "SMTMasker.h"
#include "Verifier.h"
#include <set>

using namespace clang;
extern std::string G_CryptoFuncName;
extern std::vector<std::string> G_ArgClasses;

MaskingASTVisitor::MaskingASTVisitor(ASTContext &Context, Rewriter &R)
    : Context(Context), TheRewriter(R), RandomVarCounter(0), VarFinder(), Masker() {}

bool MaskingASTVisitor::VisitFunctionDecl(FunctionDecl *F) {
    if (F->getNameAsString() == G_CryptoFuncName) {
        InCryptoFunction = true;
        if (F->hasBody()) {
            VarFinder.TraverseDecl(F);
            SensitiveVars = VarFinder.getSensitiveVars();
        }
    }
    return true;
}


bool MaskingASTVisitor::VisitBinaryOperator(BinaryOperator *Op) {
    if (!InCryptoFunction) return true; // Only apply masking inside the crypto function

    Expr *LHS = Op->getLHS()->IgnoreParenImpCasts();
    Expr *RHS = Op->getRHS()->IgnoreParenImpCasts();

    bool LHSIsSensitive = isSensitive(LHS);
    bool RHSIsSensitive = isSensitive(RHS);

    if (LHSIsSensitive || RHSIsSensitive) {
        if (Op->isAdditiveOp() || Op->getOpcode() == BO_Xor) {
            maskLinearOperation(Op);
        } else {
            // Nonlinear, need SMT-based synthesis
            std::string maskedCode = Masker.maskNonlinearOperation(Op, RandomVarCounter);
            TheRewriter.ReplaceText(Op->getSourceRange(), maskedCode);
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

void MaskingASTVisitor::maskLinearOperation(BinaryOperator *Op) {
    // Simple linear masking:
    std::string RandomVarName = "r" + std::to_string(RandomVarCounter++);
    std::string RandomVarDecl = "bool " + RandomVarName + " = get_random();\n";

    TheRewriter.InsertTextBefore(Op->getExprLoc(), RandomVarDecl);

    std::string LHSCode = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string RHSCode = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());

    if (isSensitive(Op->getLHS())) {
        std::string MaskedLHS = "(" + LHSCode + " ^ " + RandomVarName + ")";
        TheRewriter.ReplaceText(Op->getLHS()->getSourceRange(), MaskedLHS);
    }

    if (isSensitive(Op->getRHS())) {
        std::string MaskedRHS = "(" + RHSCode + " ^ " + RandomVarName + ")";
        TheRewriter.ReplaceText(Op->getRHS()->getSourceRange(), MaskedRHS);
    }
}

MaskingASTConsumer::MaskingASTConsumer(Rewriter &R)
    : TheRewriter(R), Visitor(nullptr) {}

void MaskingASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    Visitor = new MaskingASTVisitor(Context, TheRewriter);
    Visitor->TraverseDecl(Context.getTranslationUnitDecl());
    delete Visitor;
}
