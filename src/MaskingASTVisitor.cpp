#include "MaskingASTVisitor.h"
#include "Verifier.h"        // (If you still need the verifier, keep it)
#include <set>
#include <sstream>

using namespace clang;
extern std::string G_CryptoFuncName;
extern std::vector<std::string> G_ArgClasses;

MaskingASTVisitor::MaskingASTVisitor(ASTContext &Context, Rewriter &R)
    : Context(Context), TheRewriter(R), RandomVarCounter(0), VarFinder() {
    // Nothing special in constructor
}

// -----------------------------------------------------------
// Top-level function entry: record which variables are sensitive
// -----------------------------------------------------------
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

// -----------------------------------------------------------
// For each BinaryOperator inside the crypto function, decide
// if we need to apply masking transformations.
// -----------------------------------------------------------
bool MaskingASTVisitor::VisitBinaryOperator(BinaryOperator *Op) {
    if (!InCryptoFunction) return true; // Only mask inside the crypto function

    Expr *LHS = Op->getLHS()->IgnoreParenImpCasts();
    Expr *RHS = Op->getRHS()->IgnoreParenImpCasts();

    bool LHSIsSensitive = isSensitive(LHS);
    bool RHSIsSensitive = isSensitive(RHS);

    // If neither side is sensitive, do nothing
    if (!LHSIsSensitive && !RHSIsSensitive) {
        return true;
    }

    // If the opcode is XOR or + (which we treat as linear w.r.t. XOR),
    // apply a simpler linear masking transformation.
    if (Op->isAdditiveOp() || Op->getOpcode() == BO_Xor) {
        maskLinearOperation(Op, LHSIsSensitive, RHSIsSensitive);
    }
    // If the opcode is AND (nonlinear), apply a standard masked AND gadget
    else if (Op->getOpcode() == BO_And) {
        maskNonlinearAND(Op, LHSIsSensitive, RHSIsSensitive);
    }
    // For other nonlinear ops (like OR, etc.), you would add further gadgets
    // or a fallback approach.  For demonstration, we do an OR version:
    else if (Op->getOpcode() == BO_Or) {
        // Example of a masked OR based on De Morgan or a direct gadget
        maskNonlinearOR(Op, LHSIsSensitive, RHSIsSensitive);
    }
    else {
        // Potentially you can skip or handle with a more generic approach
        // For demonstration, do nothing or replicate a fallback.
        // We leave it unmasked here (not recommended in real usage).
    }

    return true;
}

// -----------------------------------------------------------
// Decide if an expression references a 'sensitive' variable
// (based on param classes).
// -----------------------------------------------------------
bool MaskingASTVisitor::isSensitive(Expr *E) {
    if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
        VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());
        return (VD && SensitiveVars.count(VD) > 0);
    }
    return false;
}

// -----------------------------------------------------------
// Linear masking transformation (simple).
// Insert a random variable, and replace the usage as (X ^ r).
// This is purely illustrative: real share-based code would do more.
// -----------------------------------------------------------
void MaskingASTVisitor::maskLinearOperation(BinaryOperator *Op,
                                            bool LHSIsSensitive,
                                            bool RHSIsSensitive) {
    // Generate one fresh random for LHS if needed and one for RHS if needed
    // (In practice, you'd do share-based transformations, but here we keep it simple).
    if (LHSIsSensitive) {
        std::string rNameL = createRandomVarDecl();
        // Replace LHS with (LHS ^ rNameL)
        std::string LHSCode = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
        std::string maskedLHS = "(" + LHSCode + " ^ " + rNameL + ")";
        TheRewriter.ReplaceText(Op->getLHS()->getSourceRange(), maskedLHS);
    }
    if (RHSIsSensitive) {
        std::string rNameR = createRandomVarDecl();
        // Replace RHS with (RHS ^ rNameR)
        std::string RHSCode = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());
        std::string maskedRHS = "(" + RHSCode + " ^ " + rNameR + ")";
        TheRewriter.ReplaceText(Op->getRHS()->getSourceRange(), maskedRHS);
    }
}

// -----------------------------------------------------------
// Basic first-order masked AND gadget for 1-bit boolean ops:
//   Let x, y be the original bits (potentially masked).
//   Introduce fresh random r:
//     out1 = (x1 & y1) ^ r
//     out2 = (x2 & y2) ^ (x1 & y2) ^ (x2 & y1) ^ r
//   For demonstration, we do a simple inline approach.
//
// NOTE: For multi-bit variables, you'd do this per bit or expand further.
// -----------------------------------------------------------
void MaskingASTVisitor::maskNonlinearAND(BinaryOperator *Op,
                                         bool LHSIsSensitive,
                                         bool RHSIsSensitive) {
    // 1) Create a random var
    std::string rVar = createRandomVarDecl();

    // 2) Get textual expansions of LHS, RHS
    std::string LHSCode = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string RHSCode = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());

    // 3) Build replacement code using stringstream for clarity
    std::ostringstream oss;
    oss << "{ "
        << "bool _t1 = (" << LHSCode << " & " << RHSCode << ") ^ " << rVar << "; "
        << "bool _t2 = " << rVar << "; "
        << OpSeparator() << "(_t1 ^ _t2); }";

    std::string newCode = oss.str();

    // Replace the entire operator expression with that block
    TheRewriter.ReplaceText(Op->getSourceRange(), newCode);
}

// -----------------------------------------------------------
// Example of a masked OR. Real logic might rely on
// De Morgan's OR = ~ (~x & ~y) and a masked AND gadget, etc.
// We'll just show a minimal placeholder for demonstration.
// -----------------------------------------------------------
void MaskingASTVisitor::maskNonlinearOR(BinaryOperator *Op,
                                        bool LHSIsSensitive,
                                        bool RHSIsSensitive) {
    std::string rVar = createRandomVarDecl();
    std::string LHSCode = TheRewriter.getRewrittenText(Op->getLHS()->getSourceRange());
    std::string RHSCode = TheRewriter.getRewrittenText(Op->getRHS()->getSourceRange());

    std::ostringstream oss;
    oss << "{ "
        << "bool _rTemp = " << rVar << "; "
        << "bool _t1 = (" << LHSCode << " | " << RHSCode << ") ^ _rTemp; "
        << "bool _t2 = _rTemp; "
        << OpSeparator() << "(_t1 ^ _t2); }";

    std::string newCode = oss.str();

    TheRewriter.ReplaceText(Op->getSourceRange(), newCode);
}

// -----------------------------------------------------------
// Helper to create a fresh random variable declaration at
// the current expression location, returning the var name.
// -----------------------------------------------------------
std::string MaskingASTVisitor::createRandomVarDecl() {
    std::string varName = "r" + std::to_string(RandomVarCounter++);
    std::string decl = "bool " + varName + " = get_random();\n";
    // Insert the declaration text right before the expression that triggered it.
    // We can insert at the current Rewriter location or store it for insertion.
    // For simplicity, we do it at the translation unit start:
    // (Alternatively, we could do InsertTextBefore at some known location.)
    FileID mainFileID = TheRewriter.getSourceMgr().getMainFileID();
    TheRewriter.InsertTextBefore(
        TheRewriter.getSourceMgr().getLocForStartOfFile(mainFileID),
        decl);
    return varName;
}

// Just a small helper to produce a semicolon or operator separator
std::string MaskingASTVisitor::OpSeparator() {
    // We can just return "; "
    return "; ";
}

// -----------------------------------------------------------
// MaskingASTConsumer
// -----------------------------------------------------------
MaskingASTConsumer::MaskingASTConsumer(Rewriter &R)
    : TheRewriter(R), Visitor(nullptr) {}

void MaskingASTConsumer::HandleTranslationUnit(ASTContext &Context) {
    Visitor = new MaskingASTVisitor(Context, TheRewriter);
    Visitor->TraverseDecl(Context.getTranslationUnitDecl());
    delete Visitor;
}
