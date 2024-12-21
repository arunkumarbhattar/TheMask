#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
   A single get_random() function returning a boolean
   0 or 1.
*/
static inline bool get_random(void) {
    return (rand() & 1) ? true : false;
}

/*
   A toy "masked AND":
   out = x AND y
   We produce out as (z1 ^ z2) where z1,z2 are shares.
   For demonstration, each gadget call is for a single bit.
*/
bool masked_AND(bool x, bool y) {
    bool r = get_random();      // random bit
    bool z1 = (x & y) ^ r;      // first share
    bool z2 = r;                // second share
    return (z1 ^ z2);           // out = z1 ^ z2
}

/*
   A toy "masked OR" gadget:
   out = x OR y
   Similarly returning (z1 ^ z2).
*/
bool masked_OR(bool x, bool y) {
    bool r = get_random();
    bool z1 = (x | y) ^ r;
    bool z2 = r;
    return (z1 ^ z2);
}

/*
   A bigger "cryptoFunc" that uses both linear (XOR, +) and
   masked nonlinear ops (masked_AND, masked_OR).
*/
int cryptoFunc(int i1, int k, int i2) {
    // Convert to booleans for demonstration
    bool x = (bool)i1;
    bool y = (bool)k;
    bool z = (bool)i2;

    // Linear combination:
    // s1 = x ^ y
    bool s1 = (x ^ y);

    // Another linear step:
    // s2 = s1 + z   (in boolean context, + is effectively ^ if 0/1, but let's keep it)
    bool s2 = ((int)s1 + (int)z) & 1;  // just a bit example

    // Nonlinear masked step:
    // s3 = masked_AND(s2, x)
    bool s3 = masked_AND(s2, x);

    // Another masked nonlinear step:
    // s4 = masked_OR(s1, s2)
    bool s4 = masked_OR(s1, s2);

    // Final combination
    // s5 = s3 ^ s4
    bool s5 = s3 ^ s4;

    // Return s5 as an integer
    return (int) s5;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <i1> <k> <i2>\n", argv[0]);
        return 1;
    }

    // We read them as bool, but they can be 0/1 or any integer
    bool i1 = (bool)atoi(argv[1]);
    bool k  = (bool)atoi(argv[2]);
    bool i2 = (bool)atoi(argv[3]);

    bool result = (bool)cryptoFunc(i1, k, i2);
    printf("Crypto result: %d\n", (int)result);
    return 0;
}
