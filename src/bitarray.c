// src/bitarray.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitarray.h"

const char HEXALPHABET[16] = "0123456789abcdef";
const char B64ALPHABET[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

long balen (bitarray* ba) {
    return ba->len;
}

void printascii(bitarray* ba) {
    long i;
    printf("ascii: ");
    for (i = 0; i < balen(ba); i++) {
        if (ba->byte[i] >= 32 && ba->byte[i] <= 126) printf("%c", ba->byte[i]);
        else printf(" ");
//      else printf("'\\%d'", ba->byte[i]);
    }
    printf("\n");
}

void printhex(bitarray* ba) {
    long i;
    printf("hex: ");
    for (i = 0; i < balen(ba); i++) printf("%02x", ba->byte[i]);
    printf("\n");
}

void print64(bitarray* ba) {
    long i, j, res;
    printf("B64: ");
    for (i = 0; i < balen(ba) * 8; i += 6) {
        res = 0;
        for (j = 0; j < 6; j++) {
            res <<= 1;
            res += (ba->byte[(i + j) / 8] >> (7 - (i + j) % 8) & 1);
        }
        printf("%c", B64ALPHABET[res]);
    }
    printf("\n");
}

void printall(bitarray* ba) {
    printf("length = %ld\n", balen(ba));
    printhex(ba);
    printascii(ba);
    print64(ba);
}

bitarray* new_ba(long bytearraylength) {
    long i;
    bitarray* res = malloc(sizeof(bitarray) + bytearraylength); // Only a single malloc needed because of the "struct hack"
    res->len = bytearraylength;
    for (i = 0; i < res->len; i++) res->byte[i] = 0;
    return res;
}

void destroy_ba(bitarray* ba) {
    free(ba); // Still enough because of the struct hack
}

unsigned char hexnumvalue(char hexchar) {
    //assumes a clean hexchar
    unsigned char i;
    for (i = 0; i < 16; i++) {
        if (hexchar == HEXALPHABET[i]) break;
    }
    return i;
}

unsigned char b64numvalue(char b64char) {
    // assumes a clean b64char
    unsigned char i;
    for (i = 0; i < 64; i++) {
        if (b64char == B64ALPHABET[i]) break;
    }
    return i;
}

bitarray* create_ba_from_hex(char* hexstr) {
    // Assumes a clean hexstring, no line feeds
    long i;
    bitarray* res = new_ba(strlen(hexstr) / 2);
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = hexnumvalue(hexstr[2*i]) * 16 + hexnumvalue(hexstr[2*i + 1]);
    }
    return res;
}

bitarray* create_ba_from_64(char *b64str) {
    // Assumes a clean b64string, no line feeds
    long i, leftshift, rightshift;
    bitarray* res = new_ba(((strlen(b64str) - 1) * 3 / 4) + 1);
    for (i = 0; i < balen(res) - 1; i++) {
        leftshift = ((i * 2 + 2) % 6); if (leftshift == 0) leftshift = 6;
        rightshift = (6 - ((i * 2 + 2) % 6)); if (rightshift == 6) rightshift = 0;
        res->byte[i] = (b64numvalue(b64str[i * 4 / 3]) << leftshift) + (b64numvalue(b64str[i * 4 / 3 + 1]) >> rightshift);
    }
    leftshift = ((i * 2 + 2) % 6); if (leftshift == 0) leftshift = 6;
    rightshift = (6 - ((i * 2 + 2) % 6)); if (rightshift == 6) rightshift = 0;
    res->byte[i] = (b64numvalue(b64str[i * 4 / 3]) << leftshift) + (b64numvalue(b64str[i * 4 / 3 + 1]) >> rightshift);
    return res;
}

bitarray* create_ba_from_ascii(char*str) { 
    long i;
    bitarray* res = new_ba(strlen(str));
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = str[i];
    }
    return res;
}

void update_ba_from_hex(bitarray* ba, char* hexstr) {
    long i;
    for (i = 0; i < balen(ba); i++) {
        ba->byte[i] = hexnumvalue(hexstr[2*i]) * 16 + hexnumvalue(hexstr[2*i + 1]);
    }
}

void update_ba_from_ascii(bitarray* ba, char* str) {
    long i;
    for (i = 0; i < balen(ba); i++) {
        ba->byte[i] = str[i];
    }
}

void bacopy(bitarray* dest, bitarray* src) {
    long i;
    dest->len = src->len;
    for (i = 0; i < balen(src); i++) {
        dest->byte[i] = src->byte[i];
    }
}

int isequalba(bitarray* ba1, bitarray* ba2) {
    long i;
    if (balen(ba1) == balen(ba2)) {
        for (i = 0; i < balen(ba1); i++) {
            if (ba1->byte[i] != ba2-> byte[i]) return 0;
        }
        return 1;
    }
    return 0;
}

void baxor(bitarray *res, bitarray *ba1, bitarray *ba2) {
    // ba1 should be larger or equal to ba2
    // If ba2 is shorter than ba1, xor is continued with the first byte of ba2
    // res should be created before calling this function, and with length equal to ba1
    long i;
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = ba1->byte[i] ^ ba2->byte[i % ba2->len];
    }
}

