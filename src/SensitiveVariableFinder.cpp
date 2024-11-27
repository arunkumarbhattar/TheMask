// src/SensitiveVariableFinder.cpp
#include "SensitiveVariableFinder.h"

using namespace clang;

SensitiveVariableFinder::SensitiveVariableFinder() {}

bool SensitiveVariableFinder::VisitVarDecl(VarDecl *Decl) {
    std::string varName = Decl->getNameAsString();
    if (varName.find("key") != std::string::npos || varName == "k") {
        SensitiveVars.insert(Decl);
    }
    return true;
}

bool SensitiveVariableFinder::isSensitive(Expr *E) {
    if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
        VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());
        return SensitiveVars.count(VD) > 0;
    }
    return false;
}

std::set<VarDecl *> SensitiveVariableFinder::getSensitiveVars() {
    return SensitiveVars;
}
