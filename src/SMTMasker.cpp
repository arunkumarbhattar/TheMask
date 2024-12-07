#include "SMTMasker.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>

SMTMasker::SMTMasker() {}

std::string SMTMasker::maskNonlinearOperation(clang::BinaryOperator *Op, int &RandomVarCounter) {
    // Generate SMT input file for nonlinear masking
    std::string smtInput = generateSMTInput(Op);

    // Call SMT solver
    std::string smtOutput = callSMTSolver(smtInput);

    // Parse SMT output to get masked operation
    std::string maskedCode = parseSMTOutput(smtOutput);

    // Update RandomVarCounter if we introduced random bits: we introduced 2 random bits (r1,r2)
    RandomVarCounter += 2;

    return maskedCode;
}

std::string SMTMasker::generateSMTInput(clang::BinaryOperator *Op) {
    // We assume Op is a nonlinear operation (e.g., AND).
    // Let's identify the target function f(i1,i2) = i1 Op i2.
    // For demonstration, we consider that we know it's AND.
    // If you need to support other ops, add conditions or detect Op->getOpcode().
    // Here we just assume AND for a nonlinear example.

    // We want:
    // fp(i1⊕r1, i2⊕r2) ⊕ fpp(i1⊕r1, i2⊕r2) = i1 AND i2
    //
    // We'll create boolean variables to pick operations for fp and fpp from {AND, OR, XOR}.
    // Let fp = op1(a,b), fpp = op2(c,d) where a,b,c,d are chosen from {i1⊕r1, i2⊕r2}.
    // We'll enforce that fp and fpp each use both (i1⊕r1) and (i2⊕r2) in some order.
    //
    // To represent choices, we have boolean vars:
    // op_fp_and, op_fp_or, op_fp_xor (exactly one true)
    // op_fpp_and, op_fpp_or, op_fpp_xor (exactly one true)
    //
    // We'll test all 16 combinations of i1,i2,r1,r2 to ensure correctness.
    // If solver finds a model, we parse it and generate code.

    std::ostringstream oss;
    oss << "(set-logic QF_BV)\n";
    oss << "(set-option :produce-models true)\n";
    // Declare boolean variables for inputs and random bits
    oss << "(declare-fun i1 () Bool)\n";
    oss << "(declare-fun i2 () Bool)\n";
    oss << "(declare-fun r1 () Bool)\n";
    oss << "(declare-fun r2 () Bool)\n";

    // Define masked inputs:
    oss << "(define-fun x1 () Bool (xor i1 r1))\n";
    oss << "(define-fun x2 () Bool (xor i2 r2))\n";

    // Operation choice for fp:
    oss << "(declare-fun op_fp_and () Bool)\n";
    oss << "(declare-fun op_fp_or () Bool)\n";
    oss << "(declare-fun op_fp_xor () Bool)\n";

    // Exactly one operation for fp:
    oss << "(assert (= 1 (+ (ite op_fp_and 1 0) (ite op_fp_or 1 0) (ite op_fp_xor 1 0))))\n";

    // Operation choice for fpp:
    oss << "(declare-fun op_fpp_and () Bool)\n";
    oss << "(declare-fun op_fpp_or () Bool)\n";
    oss << "(declare-fun op_fpp_xor () Bool)\n";
    oss << "(assert (= 1 (+ (ite op_fpp_and 1 0) (ite op_fpp_or 1 0) (ite op_fpp_xor 1 0))))\n";

    // fp and fpp must each depend on x1 and x2. We'll just fix that fp uses (x1,x2) and fpp uses (x1,x2).
    // fp = chosen_op_fp(x1,x2)
    oss << "(define-fun fp () Bool (ite op_fp_and (and x1 x2) (ite op_fp_or (or x1 x2) (xor x1 x2))))\n";
    // fpp = chosen_op_fpp(x1,x2)
    oss << "(define-fun fpp() Bool (ite op_fpp_and (and x1 x2) (ite op_fpp_or (or x1 x2) (xor x1 x2))))\n";

    // out = fp ⊕ fpp
    oss << "(define-fun out() Bool (xor fp fpp))\n";

    // We want out = i1 AND i2 for all i1,i2,r1,r2 in {0,1}.
    // We'll add assertions for all combinations. That's 16 constraints.
    // We'll instantiate i1,i2,r1,r2 as constants in assertions.

    // Helper: define a function to assert equality under specific assignments:
    // We'll just inline them:
    // For each combination of i1,i2,r1,r2:
    // (push)
    // (assert (= i1 false)) or true
    // similarly for i2,r1,r2
    // (assert (= out (and i1 i2)))
    // (check-sat)
    // But we want a single solve. So we must encode them all simultaneously.
    // We'll do this by universal encoding:
    // Actually we can do a big conjunction by enumerating all cases and forcing out to equal (and i1 i2) in each case simultaneously.
    // We'll introduce fresh variables for each pattern and assert them combined.

    // Trick: We'll represent all patterns by if-then-else and ensure equality holds for all.

    // Another approach: encode a big AND of conditions:
    // We cannot branch easily in QF_BV on booleans. We'll replicate the condition by quantifier expansion:
    // Actually, we can do a known trick: For each of the 16 assignments, we force that if (i1,i2,r1,r2) match that assignment, then out = i1 and i2.
    // We'll do this by enumerating all assignments as separate assertions. We'll use "assert" with all combinations and rely on solver to handle it.
    // We'll just unify the technique: we have no quantifiers, so we rely on solver to solve all at once.

    // i1,i2,r1,r2 can each be true or false, we must ensure out=(and i1 i2) for all:
    // We'll do this by enumerating all combinations and assert:
    // For each combination of i1,i2,r1,r2, we force out=(and i1 i2).
    // That means for each combination:
    // (push)
    // (assert (= i1 val_i1))
    // (assert (= i2 val_i2))
    // (assert (= r1 val_r1))
    // (assert (= r2 val_r2))
    // (assert (= out (and i1 i2)))
    // We'll not pop. We'll just assert all combinations. If any combination is not satisfied, no model.

    bool bools[2] = {false,true};
    for (bool I1 : bools) {
        for (bool I2 : bools) {
            for (bool R1 : bools) {
                for (bool R2 : bools) {
                    oss << "(assert (= i1 " << (I1?"true":"false") << "))\n";
                    oss << "(assert (= i2 " << (I2?"true":"false") << "))\n";
                    oss << "(assert (= r1 " << (R1?"true":"false") << "))\n";
                    oss << "(assert (= r2 " << (R2?"true":"false") << "))\n";

                    // out must equal (and i1 i2)
                    oss << "(assert (= out (and i1 i2)))\n";
                }
            }
        }
    }

    oss << "(check-sat)\n(get-model)\n";
    return oss.str();
}

std::string SMTMasker::callSMTSolver(const std::string &smtInput) {
    std::ofstream smtFile("temp.smt2");
    smtFile << smtInput;
    smtFile.close();

    std::string command = "z3 temp.smt2 2>&1";
    std::string result;
    char buffer[128];
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
        return "ERROR";
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);

    std::remove("temp.smt2");

    return result;
}

std::string SMTMasker::parseSMTOutput(const std::string &smtOutput) {
    // Parse model to see which ops are chosen for fp and fpp.
    // We defined three boolean vars for fp: op_fp_and, op_fp_or, op_fp_xor
    // and three for fpp: op_fpp_and, op_fpp_or, op_fpp_xor
    // Exactly one of each is true.
    //
    // We'll search in the model output for lines like:
    // (define-fun op_fp_and ... true/false)
    // We'll extract which is true for fp and fpp.

    bool fp_and = false, fp_or = false, fp_xor = false;
    bool fpp_and = false, fpp_or = false, fpp_xor = false;

    {
        std::istringstream iss(smtOutput);
        std::string line;
        while (std::getline(iss,line)) {
            if (line.find("op_fp_and") != std::string::npos && line.find("true") != std::string::npos) fp_and = true;
            if (line.find("op_fp_or") != std::string::npos && line.find("true") != std::string::npos) fp_or = true;
            if (line.find("op_fp_xor") != std::string::npos && line.find("true") != std::string::npos) fp_xor = true;

            if (line.find("op_fpp_and") != std::string::npos && line.find("true") != std::string::npos) fpp_and = true;
            if (line.find("op_fpp_or") != std::string::npos && line.find("true") != std::string::npos) fpp_or = true;
            if (line.find("op_fpp_xor") != std::string::npos && line.find("true") != std::string::npos) fpp_xor = true;
        }
    }

    // Now we know which ops are used:
    // fp_op = one of {and, or, xor}
    // fpp_op = similarly

    std::string fp_op = fp_and ? "(&&" : (fp_or?"(||":"(^");
    std::string fpp_op = fpp_and ? "(&&" : (fpp_or?"(||":"(^");

    // We have:
    // fp(x1,x2) = x1 op_fp x2
    // fpp(x1,x2) = x1 op_fpp x2
    // final out = fp ⊕ fpp
    //
    // We'll produce code:
    // bool r1 = get_random();
    // bool r2 = get_random();
    // bool masked_i1 = i1 ^ r1;
    // bool masked_i2 = i2 ^ r2;
    // bool fp = (masked_i1 FP_OP masked_i2);
    // bool fpp = (masked_i1 FPP_OP masked_i2);
    // bool out = fp ^ fpp;

    // Replace (^ with XOR syntax, we used C++ styled ops:
    // We'll use '^' for xor, '&&' for and, '||' for or.
    // Actually we used "(" at start for clarity. Let's finalize proper code:

    if (fp_op=="(^") fp_op = "^";
    else if (fp_op=="(&&") fp_op="&&";
    else if (fp_op=="(||") fp_op="||";

    if (fpp_op=="(^") fpp_op = "^";
    else if (fpp_op=="(&&") fpp_op="&&";
    else if (fpp_op=="(||") fpp_op="||";

    std::ostringstream code;
    code << "{\n"
         << "  bool r1 = get_random();\n"
         << "  bool r2 = get_random();\n"
         << "  bool masked_i1 = i1 ^ r1;\n"
         << "  bool masked_i2 = i2 ^ r2;\n"
         << "  bool fp = (masked_i1 " << " " << fp_op << " " << "masked_i2);\n"
         << "  bool fpp = (masked_i1 " << " " << fpp_op << " " << "masked_i2);\n"
         << "  bool out = fp ^ fpp;\n"
         << "  out;\n"
         << "}";

    return code.str();
}
