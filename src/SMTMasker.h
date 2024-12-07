#ifndef SMTMASKER_H
#define SMTMASKER_H

#include <string>
#include "clang/AST/Expr.h"

class SMTMasker {
public:
    SMTMasker();
    std::string maskNonlinearOperation(clang::BinaryOperator *Op, int &RandomVarCounter);

private:
    std::string generateSMTInput(clang::BinaryOperator *Op);
    std::string callSMTSolver(const std::string &smtInput);
    std::string parseSMTOutput(const std::string &smtOutput);
};

#endif
