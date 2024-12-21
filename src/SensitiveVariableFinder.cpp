// src/SensitiveVariableFinder.cpp
#include "SensitiveVariableFinder.h"
#include <set>
#include "clang/AST/Decl.h"

using namespace clang;
extern std::string G_CryptoFuncName;
extern std::vector<std::string> G_ArgClasses;

SensitiveVariableFinder::SensitiveVariableFinder() {}

bool SensitiveVariableFinder::VisitVarDecl(VarDecl *Decl) {
    // Check if this VarDecl is a parameter of the cryptographic function.
    if (auto *FD = dyn_cast<FunctionDecl>(Decl->getDeclContext())) {
        if (FD->getNameAsString() == G_CryptoFuncName) {
            // Identify the index of this parameter
            for (unsigned i = 0; i < FD->getNumParams(); i++) {
                if (FD->getParamDecl(i) == Decl) {
                    // Found the matching parameter
                    std::string cls = G_ArgClasses[i];
                    if (cls == "key") {
                        // Mark as sensitive
                        SensitiveVars.insert(Decl);
                    }
                    // if cls == "public", do nothing
                    // if cls == "random", etc.
                    break;
                }
            }
        }
    }
    return true;
}

bool SensitiveVariableFinder::isSensitive(Expr *E) {
    if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
        VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());
        return (VD && SensitiveVars.count(VD) > 0);
    }
    return false;
}

std::set<VarDecl *> SensitiveVariableFinder::getSensitiveVars() {
    return SensitiveVars;
}
