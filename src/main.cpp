// src/main.cpp
#include "MaskingFrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory MaskingCategory("masking options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MaskingCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Error parsing command-line arguments: "
                     << llvm::toString(ExpectedParser.takeError()) << "\n";
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();

    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    return Tool.run(newFrontendActionFactory<MaskingFrontendAction>().get());
}
