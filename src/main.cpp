// src/main.cpp
#include "MaskingFrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory MaskingCategory("masking options");

static llvm::cl::opt<std::string> CryptoFuncName(
    "crypto-func",
    llvm::cl::desc("Name of the cryptographic function"),
    llvm::cl::Required,
    llvm::cl::cat(MaskingCategory));

static llvm::cl::list<std::string> ArgClasses(
    "arg-class",
    llvm::cl::desc("List of argument classes (key, public, random, etc.)"),
    llvm::cl::OneOrMore,
    llvm::cl::cat(MaskingCategory));

// Global config
std::string G_CryptoFuncName;
std::vector<std::string> G_ArgClasses;

int main(int argc, const char **argv) {
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, MaskingCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Error parsing command-line arguments: "
                     << llvm::toString(ExpectedParser.takeError()) << "\n";
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();

    // store config
    G_CryptoFuncName = CryptoFuncName;
    for (auto &cls : ArgClasses) {
        G_ArgClasses.push_back(cls);
    }

    // run the Clang tool
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<MaskingFrontendAction>().get());
}
