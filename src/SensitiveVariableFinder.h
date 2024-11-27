// src/SensitiveVariableFinder.h
#ifndef SENSITIVEVARIABLEFINDER_H
#define SENSITIVEVARIABLEFINDER_H

#include "clang/AST/RecursiveASTVisitor.h"
#include <set>

class SensitiveVariableFinder
    : public clang::RecursiveASTVisitor<SensitiveVariableFinder> {
public:
    SensitiveVariableFinder();

    bool VisitVarDecl(clang::VarDecl *Decl);
    bool isSensitive(clang::Expr *Expr);
    std::set<clang::VarDecl *> getSensitiveVars();

private:
    std::set<clang::VarDecl *> SensitiveVars;
};

#endif // SENSITIVEVARIABLEFINDER_H
