# MaskingRewriter: A Source-to-Source Code Masking Tool

MaskingRewriter is a source-to-source code transformation tool designed to apply masking techniques to cryptographic code, enhancing its security against side-channel attacks such as Differential Power Analysis (DPA). By analyzing the abstract syntax tree (AST) of input code, the tool identifies sensitive variables and applies appropriate masking strategies to linear and nonlinear operations.

## Table of Contents
- [Project Overview](#project-overview)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Dependencies](#dependencies)
- [Algorithmic Overview](#algorithmic-overview)
    - [Sensitive Variable Identification](#sensitive-variable-identification)
    - [Masking Linear Operations](#masking-linear-operations)
    - [Masking Nonlinear Operations](#masking-nonlinear-operations)
    - [Verification of Equivalence](#verification-of-equivalence)
- [Component Details](#component-details)
    - [main.cpp](#maincpp)
    - [MaskingFrontendAction](#maskingfrontendaction)
    - [MaskingASTVisitor](#maskingastvisitor)
    - [SensitiveVariableFinder](#sensitivevariablefinder)
    - [SMTMasker](#smtmasker)
    - [Verifier](#verifier)
- [Usage Instructions](#usage-instructions)
- [Examples](#examples)
- [Future Work](#future-work)
- [Contributing](#contributing)
- [License](#license)

---

## Project Overview

MaskingRewriter analyzes C source code to detect and mask sensitive cryptographic operations. It leverages Clang's LibTooling framework to traverse the AST, identify sensitive variables (like cryptographic keys), and transform the code to incorporate masking techniques. This enhances the security of cryptographic implementations by reducing vulnerability to side-channel attacks.

---

## Project Structure

masking_rewriter/ ├── CMakeLists.txt ├── src/ │ ├── main.cpp │ ├── MaskingFrontendAction.h │ ├── MaskingFrontendAction.cpp │ ├── MaskingASTVisitor.h │ ├── MaskingASTVisitor.cpp │ ├── SensitiveVariableFinder.h │ ├── SensitiveVariableFinder.cpp │ ├── SMTMasker.h │ ├── SMTMasker.cpp │ ├── Verifier.h │ └── Verifier.cpp └── test/ ├── input.c └── expected_output.c


- **CMakeLists.txt**: Build configuration file.
- **src/**: Source code directory containing all implementation files.
- **test/**: Directory containing test input files for demonstration and testing.

---

## Build Instructions

### Prerequisites
- **C++17 Compiler**: GCC 7+ or Clang 5+.
- **CMake**: Version 3.10 or higher.
- **LLVM and Clang Libraries**: Version 9 or higher.
- **Z3 SMT Solver**: For handling nonlinear masking.

### Installing Dependencies
#### On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake llvm-12-dev clang-12-dev libz3-dev
```

On Other Systems:
```
    Install LLVM and Clang from official sources.
    Install Z3 SMT Solver from the Z3 GitHub repository.
```

Building the Project

```    Clone the Repository

    git clone https://github.com/yourusername/masking_rewriter.git
    cd masking_rewriter
```
Create Build Directory

```mkdir build
cd build
```
Run CMake

```cmake -DCMAKE_BUILD_TYPE=Release ..
```
Build the Project

```    make
```
    After successful compilation, the masking_rewriter executable will be available in the build directory.

Dependencies

    LLVM and Clang Libraries: Used for parsing and analyzing C code.
    Z3 SMT Solver: Used for masking nonlinear operations via SMT solving.
    C++17 Standard Library: Requires features from C++17.

Algorithmic Overview
Sensitive Variable Identification

    Detects sensitive variables by scanning for patterns (e.g., names containing "key" or named "k").
    Flags these variables as sensitive for masking.

Masking Linear Operations

    Generates a random mask.
    Applies the mask using bitwise operations.
    Preserves original computation while hiding sensitive data.

Masking Nonlinear Operations

    Converts operations to SMT-LIB format.
    Synthesizes a masked equivalent using Z3 SMT solver.
    Replaces original code with the masked version.

Verification of Equivalence

    Compiles and runs both original and transformed code with test inputs.
    Compares outputs to ensure functional equivalence.

Component Details
main.cpp

    Purpose: Entry point of the application.
    Functionality:
        Parses command-line arguments using CommonOptionsParser.
        Runs MaskingFrontendAction to initiate code transformation.

MaskingFrontendAction

    Purpose: Manages the Clang tool's frontend action.
    Files: MaskingFrontendAction.h, MaskingFrontendAction.cpp.

MaskingASTVisitor

    Purpose: Traverses the AST to find and transform sensitive operations.
    Files: MaskingASTVisitor.h, MaskingASTVisitor.cpp.

SensitiveVariableFinder

    Purpose: Identifies sensitive variables in the code.
    Files: SensitiveVariableFinder.h, SensitiveVariableFinder.cpp.

SMTMasker

    Purpose: Handles nonlinear operation masking via SMT solving.
    Files: SMTMasker.h, SMTMasker.cpp.

Verifier

    Purpose: Verifies functional equivalence between original and transformed code.
    Files: Verifier.h, Verifier.cpp.

Usage Instructions

    Prepare Input Code: Ensure you have a C source file (e.g., input.c).
    Run the Masking Rewriter:

    ./masking_rewriter input.c > masked_output.c

    Review the Transformed Code: Inspect masked_output.c for applied masking.
    Compile and Test: Test the masked code for functional correctness.
    Verify Equivalence (Optional): Use the Verifier class for automated checks.

Examples
Example Input (input.c)
```
#include <stdio.h>
#include <stdlib.h>

int crypto_function(int x, int k) {
    int y;
    y = (x & k) ^ k;
    return y;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s x k\n", argv[0]);
        return 1;
    }
    int x = atoi(argv[1]);
    int k = atoi(argv[2]);
    int y = crypto_function(x, k);
    printf("%d\n", y);
    return 0;
}
```

Example Output (masked_output.c)
```
#include <stdio.h>
#include <stdlib.h>

int crypto_function(int x, int k) {
    int y;
    int r0 = get_random();
    y = ((x ^ r0) & (k ^ r0)) ^ k;
    return y;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s x k\n", argv[0]);
        return 1;
    }
    int x = atoi(argv[1]);
    int k = atoi(argv[2]);
    int y = crypto_function(x, k);
    printf("%d\n", y);
    return 0;
}
```
Future Work

    Enhanced detection of sensitive variables.
    Support for additional data types (e.g., floating-point).
    Optimized nonlinear masking algorithms.
    Integration with build systems.
    Comprehensive testing suite.

Contributing

    Fork the repository.
    Create a branch:

git checkout -b feature/your-feature-name

Commit changes:

git commit -am 'Add new feature'

Push to your fork:

    git push origin feature/your-feature-name

    Open a pull request.

License

This project is licensed under the MIT License.

Note: This tool is intended for educational and research purposes. Ensure compliance with relevant laws and regulations when using cryptographic tools and techniques.