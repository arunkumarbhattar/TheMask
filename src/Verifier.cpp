// src/Verifier.cpp
#include "Verifier.h"
#include <fstream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <cstdlib>

Verifier::Verifier() {}

bool Verifier::verifyEquivalence(const std::string &originalCode, const std::string &transformedCode) {
    // Generate test inputs
    std::vector<std::string> testInputs = {"5 3", "10 2", "7 7"};

    for (const auto &input : testInputs) {
        std::string outputOriginal, outputTransformed;

        if (!compileAndRun(originalCode, input, outputOriginal))
            return false;
        if (!compileAndRun(transformedCode, input, outputTransformed))
            return false;

        if (outputOriginal != outputTransformed)
            return false;
    }
    return true;
}

bool Verifier::compileAndRun(const std::string &code, const std::string &input, std::string &output) {
    // Write code to temporary file
    std::ofstream codeFile("temp_code.c");
    codeFile << code;
    codeFile.close();

    // Compile the code
    std::string compileCommand = "gcc temp_code.c -o temp_program";
    int compileResult = system(compileCommand.c_str());
    if (compileResult != 0)
        return false;

    // Run the program with input
    std::string runCommand = "./temp_program " + input + " > temp_output.txt";
    int runResult = system(runCommand.c_str());
    if (runResult != 0)
        return false;

    // Read the output
    std::ifstream outputFile("temp_output.txt");
    std::stringstream buffer;
    buffer << outputFile.rdbuf();
    output = buffer.str();

    // Clean up temporary files
    std::remove("temp_code.c");
    std::remove("temp_program");
    std::remove("temp_output.txt");

    return true;
}
