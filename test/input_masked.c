#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// 32-bit random generator (placeholder).
static inline uint32_t get_random32(void) {
    // Real usage: cryptographically secure RNG.
    uint32_t r = ((uint32_t)rand() << 1) ^ (uint32_t)(rand() & 1);
    return r;
}

// "Masked32" => two 32-bit shares
typedef struct {
    uint32_t s1;
    uint32_t s2;
} Masked32;

// Create from unmasked int
static inline Masked32 mask32_create(uint32_t x) {
    Masked32 m;
    m.s2 = get_random32();
    m.s1 = x ^ m.s2;
    return m;
}

// Demask => x = s1 ^ s2
static inline uint32_t mask32_demask(Masked32 m) {
    return (m.s1 ^ m.s2);
}

// Gadgets
static inline Masked32 mask32_xor(Masked32 x, Masked32 y) {
    Masked32 out;
    out.s1 = x.s1 ^ y.s1;
    out.s2 = x.s2 ^ y.s2;
    return out;
}

static inline Masked32 mask32_and(Masked32 x, Masked32 y) {
    uint32_t r = get_random32();
    uint32_t t1 = x.s1 & y.s1;
    Masked32 out;
    out.s1 = t1 ^ r;
    out.s2 = r ^ (x.s1 & y.s2) ^ (x.s2 & y.s1) ^ (x.s2 & y.s2);
    return out;
}

static inline Masked32 mask32_or(Masked32 x, Masked32 y) {
    uint32_t r = get_random32();
    uint32_t t1 = x.s1 | y.s1;
    Masked32 out;
    out.s1 = t1 ^ r;
    out.s2 = r ^ (x.s1 | y.s2) ^ (x.s2 | y.s1) ^ (x.s2 | y.s2);
    return out;
}

static inline Masked32 mask32_add(Masked32 x, Masked32 y) {
    // naive ignoring carry leak
    uint32_t r = get_random32();
    Masked32 out;
    out.s1 = (x.s1 + y.s1) ^ r;
    out.s2 = (x.s2 + y.s2) ^ r;
    return out;
}

static inline Masked32 mask32_shiftLeft(Masked32 x, uint32_t shift) {
    Masked32 out;
    out.s1 = x.s1 << shift;
    out.s2 = x.s2 << shift;
    return out;
}
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
Masked32 k_m = mask32_create((uint32_t)k);


    // Step 1: A basic AND with the key.
    int a = Masked32 tmpM0 = mask32_k_mnd(mask32_create((uint32_t)x), mask32_create((uint32_t)k));
(int)maskMasked32 tmpM1 = mask32_k_mor(mask32_create((uint32_t)3), mask32_create((uint32_t)e));
(int)mask32_demask(tmpM1) OR.
    int c = a | b;

    // Step 4: Another step, e.g. SHIFT or ADD
    // combined with the key in some trivial way:
    int d = Masked32 tmpM2 = mask32_k_mor(mask32_create((uint32_t)(c << 1)), mask32_create((uint32_t)k));
(int)mask32_demask(tmpM2) if (argc < 4) {
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
