#include <stdio.h>
#include <stdlib.h>

bool get_random() {
    // This is just a placeholder - in practice you would implement a secure RNG.
    return rand() % 2 == 1;
}

bool cryptoFunc(bool i1, bool k, bool i2) {
    bool out = k & i2; // Nonlinear operation involving secret key 'k'
    return out;
}

int main(int argc, char** argv) {
    bool i1 = (bool)atoi(argv[1]);
    bool k  = (bool)atoi(argv[2]);
    bool i2 = (bool)atoi(argv[3]);
    bool result = cryptoFunc(i1,k,i2);
    printf("%d\n", (int)result);
    return 0;
}
