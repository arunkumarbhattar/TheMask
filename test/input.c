// test/input.c
#include <stdio.h>

int crypto_function(int x, int k) {
    int y;
    y = (x & k) ^ k;
    return y;
}

int main(int argc, char *argv[]) {
    int x = atoi(argv[1]);
    int k = atoi(argv[2]);
    int y = crypto_function(x, k);
    printf("%d\n", y);
    return 0;
}
