// src/SensitiveVariableFinder.cpp
#include "SensitiveVariableFinder.h"
#include "clang/AST/Decl.h"

extern std::string G_CryptoFuncName;
extern std::vector<std::string> G_ArgClasses;

using namespace clang;

SensitiveVariableFinder::SensitiveVariableFinder() {}

bool SensitiveVariableFinder::VisitVarDecl(VarDecl *VD) {
    if (auto *FD = dyn_cast<FunctionDecl>(VD->getDeclContext())) {
        if (FD->getNameAsString() == G_CryptoFuncName) {
            // check param index
            for (unsigned i = 0; i < FD->getNumParams(); i++) {
                if (FD->getParamDecl(i) == VD) {
                    if (i < G_ArgClasses.size()) {
                        if (G_ArgClasses[i] == "key") {
                            SensitiveVars.insert(VD);
                        }
                    }
                    break;
                }
            }
        }
    }
    return true;
}

bool SensitiveVariableFinder::isSensitive(Expr *E) {
    if (auto *DRE = dyn_cast<DeclRefExpr>(E)) {
        if (auto *VD = dyn_cast<VarDecl>(DRE->getDecl())) {
            return (SensitiveVars.count(VD) > 0);
        }
    }
    return false;
}

std::set<VarDecl *> SensitiveVariableFinder::getSensitiveVars() {
    return SensitiveVars;
}
