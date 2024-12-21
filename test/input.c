#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// We assume this file is "test/input.c"

// A "crypto" function that takes 3 parameters:
//   1) x (public)
//   2) k (key - sensitive)
//   3) z (public)
//
// It does multiple operations: AND, XOR, SHIFT, ADD, etc.
// so there's a bit more complexity than a single line.
int crypto_function(int x, int k, int z) {
    // Step 1: A basic AND with the key.
    int a = x & k;

    // Step 2: Some linear combination with the key.
    int b = (x ^ k) + z;

    // Step 3: Non-linear step combining "a" and "b":
    // let's do a naive OR.
    int c = a | b;

    // Step 4: Another step, e.g. SHIFT or ADD
    // combined with the key in some trivial way:
    int d = (c << 1) ^ k;

    // Step 5: Return final value
    return d;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <x> <k> <z>\n", argv[0]);
        return 1;
    }

    int x = atoi(argv[1]);  // public
    int k = atoi(argv[2]);  // key
    int z = atoi(argv[3]);  // public

    int y = crypto_function(x, k, z);
    printf("%d\n", y);

    return 0;
}
