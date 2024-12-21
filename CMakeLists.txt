cmake_minimum_required(VERSION 3.10)
project(MaskingRewriter)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED)
find_package(Z3 REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Define LLVM components you need
llvm_map_components_to_libnames(LLVM_LIBS
        Support
        Core
        IRReader
        # Add other required components here
)

set(SOURCE_FILES
        src/main.cpp
        src/MaskingASTVisitor.cpp
        src/MaskingFrontendAction.cpp
        src/SensitiveVariableFinder.cpp
        src/Verifier.cpp
)

add_executable(masking_rewriter ${SOURCE_FILES})

# Use target_include_directories and target_compile_definitions
target_include_directories(masking_rewriter PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
        ${Z3_INCLUDE_DIRS}
)

target_compile_definitions(masking_rewriter PRIVATE
        ${LLVM_DEFINITIONS}
)

# Link against Clang and Z3 libraries without explicitly linking LLVM
target_link_libraries(masking_rewriter
        clangTooling
        clangBasic
        clangASTMatchers
        clangAST
        clangRewrite
        ${Z3_LIBRARIES}
)

# Optionally, set compile features
target_compile_features(masking_rewriter PRIVATE cxx_std_17)
