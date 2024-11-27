// src/Verifier.h
#ifndef VERIFIER_H
#define VERIFIER_H

#include <string>

class Verifier {
public:
    Verifier();

    bool verifyEquivalence(const std::string &originalCode, const std::string &transformedCode);

private:
    // Helper functions
    bool compileAndRun(const std::string &code, const std::string &input, std::string &output);
};

#endif // VERIFIER_H
