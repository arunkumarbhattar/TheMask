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

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${Z3_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Remove llvm_map_components_to_libnames to avoid overlapping libraries
# llvm_map_components_to_libnames(LLVM_LIBS
#         Support
#         Core
#         IRReader
# )

set(SOURCE_FILES
        src/main.cpp
        src/MaskingASTVisitor.cpp
        src/MaskingFrontendAction.cpp
        src/SensitiveVariableFinder.cpp
        src/SMTMasker.cpp
        src/Verifier.cpp
)

add_executable(masking_rewriter ${SOURCE_FILES})

# Link against LLVM and essential Clang libraries
target_link_libraries(masking_rewriter
        LLVM
        clangTooling
        clangBasic
        clangASTMatchers
        clangAST
        clangRewrite
        ${Z3_LIBRARIES}
)
