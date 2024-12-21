// src/MaskingASTVisitor.cpp

#include "MaskingASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include <sstream>

extern std::string G_CryptoFuncName;
extern std::vector<std::string> G_ArgClasses;

using namespace clang;

MaskingASTVisitor::MaskingASTVisitor(ASTContext &C, Rewriter &R)
    : Context(C), TheRewriter(R) {}

bool MaskingASTVisitor::VisitFunctionDecl(FunctionDecl *F) {
    if (F->getNameAsString() == G_CryptoFuncName) {
        InCryptoFunction = true;
        if (F->hasBody()) {
            VarFinder.TraverseDecl(F);
            SensitiveVars = VarFinder.getSensitiveVars();
            splitKeyParams(F);
        }
    } else {
        InCryptoFunction = false;
    }
    return true;
}

// For each param that is "key," define "Masked32 param_m = mask32_create((uint32_t)param);"
void MaskingASTVisitor::splitKeyParams(FunctionDecl *F) {
    CompoundStmt *Body = dyn_cast<CompoundStmt>(F->getBody());
    if (!Body) return;

    SourceLocation loc = Body->getLBracLoc().getLocWithOffset(1);
    std::ostringstream code;

    for (unsigned i = 0; i < F->getNumParams(); i++) {
        ParmVarDecl *P = F->getParamDecl(i);
        if (!P) continue;
        if (isSensitiveVar(P)) {
            std::string pName = P->getNameAsString();
            std::string maskedName = pName + "_m";
            code << "Masked32 " << maskedName << " = mask32_create((uint32_t)" << pName << ");\n";
            MaskedParamMap[P] = maskedName;
        }
    }
    if (!code.str().empty()) {
        TheRewriter.InsertTextAfterToken(loc, "\n" + code.str() + "\n");
    }
}

// If referencing a sensitive param, replace with the masked param name.
bool MaskingASTVisitor::VisitDeclRefExpr(DeclRefExpr *DRE) {
    if (!InCryptoFunction) return true;

    VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());
    if (!VD) return true;
    if (!isSensitiveVar(VD)) return true;

    auto it = MaskedParamMap.find(VD);
    if (it != MaskedParamMap.end()) {
        TheRewriter.ReplaceText(DRE->getSourceRange(), it->second);
    }
    return true;
}

// For each binary op referencing masked data => ephemeral gadget call
bool MaskingASTVisitor::VisitBinaryOperator(BinaryOperator *Op) {
    if (!InCryptoFunction) return true;

    Expr *LHS = Op->getLHS()->IgnoreParenImpCasts();
    Expr *RHS = Op->getRHS()->IgnoreParenImpCasts();
    bool sL = isSensitive(LHS);
    bool sR = isSensitive(RHS);
    if (!sL && !sR) return true; // no sensitive data => skip

    switch (Op->getOpcode()) {
    case BO_And: maskAND(Op); break;
    case BO_Or:  maskOR(Op); break;
    case BO_Xor: maskXOR(Op); break;
    case BO_Add: maskADD(Op); break;
    case BO_Shl: maskShiftLeft(Op); break;
    default:
        // skip or do partial
        break;
    }
    return true;
}

bool MaskingASTVisitor::isSensitive(Expr *E) {
    return VarFinder.isSensitive(E);
}
bool MaskingASTVisitor::isSensitiveVar(VarDecl *VD) {
    return (VD && SensitiveVars.count(VD) > 0);
}

// We define a lambda that wraps unmasked references with mask32_create((uint32_t)...)
static std::string fixIfUnmasked(const std::string &txt) {
    if (txt.rfind("_m") != std::string::npos) {
        return txt; // already masked var
    }
    std::ostringstream oss;
    oss << "mask32_create((uint32_t)" << txt << ")";
    return oss.str();
}

// ----------
// Gadgets
// ----------
void MaskingASTVisitor::maskAND(BinaryOperator *Op) {
    std::string lhs = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string rhs = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
    std::string callExpr = "mask32_and(" + fixIfUnmasked(lhs) + ", " + fixIfUnmasked(rhs) + ")";
    std::string newText = createEphemeralMaskCall(callExpr, Op);
    TheRewriter.ReplaceText(Op->getSourceRange(), newText);
}

void MaskingASTVisitor::maskOR(BinaryOperator *Op) {
    std::string lhs = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string rhs = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
    std::string callExpr = "mask32_or(" + fixIfUnmasked(lhs) + ", " + fixIfUnmasked(rhs) + ")";
    std::string newText = createEphemeralMaskCall(callExpr, Op);
    TheRewriter.ReplaceText(Op->getSourceRange(), newText);
}

void MaskingASTVisitor::maskXOR(BinaryOperator *Op) {
    std::string lhs = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string rhs = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
    std::string callExpr = "mask32_xor(" + fixIfUnmasked(lhs) + ", " + fixIfUnmasked(rhs) + ")";
    std::string newText = createEphemeralMaskCall(callExpr, Op);
    TheRewriter.ReplaceText(Op->getSourceRange(), newText);
}

void MaskingASTVisitor::maskADD(BinaryOperator *Op) {
    std::string lhs = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string rhs = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
    std::string callExpr = "mask32_add(" + fixIfUnmasked(lhs) + ", " + fixIfUnmasked(rhs) + ")";
    std::string newText = createEphemeralMaskCall(callExpr, Op);
    TheRewriter.ReplaceText(Op->getSourceRange(), newText);
}

void MaskingASTVisitor::maskShiftLeft(BinaryOperator *Op) {
    // (lhs << rhs)
    std::string lhs = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string rhs = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
    std::string callExpr = "mask32_shiftLeft(" + fixIfUnmasked(lhs)
                         + ", (uint32_t)(" + rhs + "))";
    std::string newText = createEphemeralMaskCall(callExpr, Op);
    TheRewriter.ReplaceText(Op->getSourceRange(), newText);
}

// createEphemeralMaskCall =>
//   1) Insert "Masked32 tmpM# = <callExpr>;" before the statement
//   2) Return "(int)mask32_demask(tmpM#)" for usage in the expression
std::string MaskingASTVisitor::createEphemeralMaskCall(const std::string &callExpr,
                                                       BinaryOperator *Op) {
    // We'll produce ephemeral var "Masked32 tmpM0 = <callExpr>;"
    std::string varName = "tmpM" + std::to_string(TempCounter++);
    std::ostringstream decl;
    decl << "Masked32 " << varName << " = " << callExpr << ";";

    // Insert just before the statement that contains 'Op'.
    // We get the beginning of that statement:
    Stmt *parentStmt = Op;
    SourceLocation startLoc = parentStmt->getBeginLoc();
    TheRewriter.InsertTextBefore(startLoc, decl.str() + "\n");

    // Then we replace the usage with: "(int)mask32_demask(tmpM#)"
    std::ostringstream usage;
    usage << "(int)mask32_demask(" << varName << ")";
    return usage.str();
}

// ASTConsumer
MaskingASTConsumer::MaskingASTConsumer(Rewriter &R)
    : TheRewriter(R), Visitor(nullptr) {}

void MaskingASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    Visitor = new MaskingASTVisitor(Context, TheRewriter);
    Visitor->TraverseDecl(Context.getTranslationUnitDecl());
    delete Visitor;
}
