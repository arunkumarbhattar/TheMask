#include "MaskingFrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;

// Define command-line options
static llvm::cl::OptionCategory MaskingCategory("masking options");

static llvm::cl::opt<std::string> CryptoFuncName(
    "crypto-func",
    llvm::cl::desc("Name of the cryptographic function"),
    llvm::cl::Required, // Must be specified
    llvm::cl::cat(MaskingCategory));

static llvm::cl::list<std::string> ArgClasses(
    "arg-class",
    llvm::cl::desc("List of argument classes (key, public, random) in order"),
    llvm::cl::OneOrMore, // At least one
    llvm::cl::cat(MaskingCategory));

// Global variables to store the configuration
std::string G_CryptoFuncName;
std::vector<std::string> G_ArgClasses;

int main(int argc, const char **argv) {
    // Parse command-line options
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, MaskingCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Error parsing command-line arguments: "
                     << llvm::toString(ExpectedParser.takeError()) << "\n";
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();

    // Store in global variables
    G_CryptoFuncName = CryptoFuncName;
    for (const auto &ArgClass : ArgClasses) {
        G_ArgClasses.push_back(ArgClass);
    }

    // Run the Clang tool
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<MaskingFrontendAction>().get());
}
