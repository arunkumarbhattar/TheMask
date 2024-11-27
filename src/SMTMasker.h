// src/SMTMasker.h
#ifndef SMTMASKER_H
#define SMTMASKER_H

#include "clang/AST/Expr.h"
#include <string>

class SMTMasker {
public:
    SMTMasker();

    std::string maskNonlinearOperation(clang::BinaryOperator *Op, int &RandomVarCounter);

private:
    // Helper functions for interacting with SMT solver
    std::string generateSMTInput(clang::BinaryOperator *Op);
    std::string callSMTSolver(const std::string &smtInput);
    std::string parseSMTOutput(const std::string &smtOutput);
};

#endif // SMTMASKER_H
