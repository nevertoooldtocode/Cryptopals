#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int sumbits(char c) {
    int i, res = 0;
    for (i = 0; i < 8; i++) {
        res += (c >> i) & 1;
    }
    return res;
}

int hamming(char *a, char *b) {
    int i, res = 0;
    for (i = 0; i < strlen(a); i++) {
        res += sumbits(a[i] ^  b[i]);
    }
    return res;
}

int main() {
    assert(hamming("this is a test", "wokka wokka!!!") == 37);
    printf("result: %d\n", hamming("this is a test", "wokka wokka!!!"));
    return 0;
}
