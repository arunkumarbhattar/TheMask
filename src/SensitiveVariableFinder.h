// src/SensitiveVariableFinder.h
#ifndef SENSITIVE_VARIABLE_FINDER_H
#define SENSITIVE_VARIABLE_FINDER_H

#include "clang/AST/RecursiveASTVisitor.h"
#include <set>

class SensitiveVariableFinder
  : public clang::RecursiveASTVisitor<SensitiveVariableFinder> {
public:
    SensitiveVariableFinder();

    bool VisitVarDecl(clang::VarDecl *Decl);
    bool isSensitive(clang::Expr *E);
    std::set<clang::VarDecl*> getSensitiveVars();

private:
    std::set<clang::VarDecl*> SensitiveVars;
};

#endif
