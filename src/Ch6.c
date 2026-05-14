// src/Ch6.c
//
// This is Cryptopals Challenge 6 from Set 1
// Break repeating-key XOR
//
// To do: put English functions in separate module
// Include histogram generating somewhere

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bitarray.h"

#define ALPHABETSIZE 26
#define WORSTINDICATOR 999999999

typedef long histogram[ALPHABETSIZE];
// To store the frequency of the 26 letters of the alphabet in a text

const histogram ENGFREQ1000 = {81,13,39,39,115,24,18,38,67,1,4,42,35,77,78,24,1,68,67,90,29,6,12,5,13,1};
// Contains average frequency of the 26 letters of the alphabet in an English technical text

void printhisto(histogram his) {
    long i;
    printf("histogram: {");
    for (i = 0; i < ALPHABETSIZE; i++) printf("%ld, ", his[i]);
    printf("}\n");
}

long englishindicator(bitarray *ba) {
    // Uses least square method comparing the argument to average English character frequency
    // The smaller the sum, the closer the character disctribution is to ENGFREQ1000
    long i, res = 0, sum = 0, penalty = 0;
    histogram histo;
    for(i = 0; i < ALPHABETSIZE; i++) histo[i] = 0;
    for (i = 0; i < ba->len; i++) {
        if (ba->byte[i] >= 65 && ba->byte[i] <= 90) histo[ba->byte[i] - 65] += 1; // Hoofdletters
        else if (ba->byte[i] >= 97 && ba->byte[i] <= 122) histo[ba->byte[i] - 97] += 1; // Kleine letters
        else penalty += 5000; // Any byte not in the alphabet incurs a penalty
    }
    for (i = 0; i < ALPHABETSIZE; i++) sum += histo[i];
    if (sum == 0) {
        res = WORSTINDICATOR;
    }
    else {
       for (i = 0; i < ALPHABETSIZE; i++) {
           res += ((histo[i] * 1000 / sum) - ENGFREQ1000[i]) * ((histo[i] * 1000 / sum) - ENGFREQ1000[i]);
       }
    }
    return res + penalty;
}

long sumbits(char c) {
    long i, res = 0;
    for (i = 0; i < 8; i++) {
        res += (c >> i) & 1;
    }
    return res;
}

long hamming(bitarray *ba1, bitarray *ba2) {
    // Computes the Hamming distance between the arguments
    // Hamming distance is the number of differing bits
    long i, res = 0;
    for (i = 0; i < balen(ba1); i++) {
        res += sumbits(ba1->byte[i] ^  ba2->byte[i]);
    }
    return res;
}

void charxor(bitarray *res, bitarray *ba, unsigned char key) {
    long i;
    for (i = 0; i < balen(res); i++) {
    res->byte[i] = ba->byte[i] ^ key;
    }
}

void findcharxorkey(bitarray *ba, long *minkey, long *minind) {
    // Tries all 256 byte-values as key for repeated xor in the bitarray
    // minkey will contain the key that produces the lowest englishindicator,
    // minind will contain the value of that indicator
    long j, ind, key;
    *minkey = 0;
    *minind = WORSTINDICATOR;
    bitarray *tempres = new_ba(balen(ba));
    bitarray *res = new_ba(balen(ba));
    for (key = 0; key < 256; key++) {
        charxor(tempres, ba, key);
        ind = englishindicator(tempres);
        if (*minind > ind) {
            *minkey = key;
            *minind = ind;
            for (j = 0; j < balen(res); j++) {
                res->byte[j] = tempres->byte[j];
            }
        }
    //printf("key = %ld, minkey = %ld, min = %ld, cur = %ld\n", key, minkey, min, cur);
    }
    //printf("Solution key = %c, english indicator = %ld, solution: ", (char)minkey, min);
    //printascii(res);
    destroy_ba(res);
    destroy_ba(tempres);
}

long hamdist(bitarray *ba, long keysize) {
    // Returns the normalized hamming distance between the first *blocknumber* consecutive pairs of length *keysize*
    // A low value is an indication that simple xor encryption was done with a key of keysize length
    long i, res = 0, blocknumber = 0;
    for (blocknumber = 0; blocknumber < 20; blocknumber++) {
        for (i = 0; i < keysize; i++) {
            res += sumbits(ba->byte[2 * blocknumber * keysize + i] ^  ba->byte[(2 * blocknumber + 1) * keysize + i]);
        }
    }
    return res * 1000 / keysize;
}

int main(int argc, char **argv) {

    FILE *stream;
    char *buf = NULL;
    size_t size = 0;
    bitarray *ba = NULL, *res = NULL, *key = NULL, *block = NULL;
    long trykeysize = 0, keysize = 0, tryhamdist = 0, minhamdist = 999999999;
    long i, j, ind, keychar;

    stream = (argc == 1) ? stdin : fopen(argv[1], "rb");

    fseek(stream, 0L, SEEK_END); // This only works with an input file without LFs
    size = ftell(stream);
    buf = malloc(size+1);
    rewind(stream);
    fread(buf, 1,size, stream);
    buf[size] = '\0';
    if (buf[size -1] == '\n') buf[size - 1] = '\0'; //get rid of linefeed added by vim

    ba = create_ba_from_64(buf);

    for (trykeysize = 2; trykeysize < 41; trykeysize++) {
//      printf("trykeysize = %ld, hamdist = %ld\n", trykeysize, hamdist(ba, trykeysize));
        tryhamdist = hamdist(ba, trykeysize);
        if (tryhamdist < minhamdist) {
            minhamdist = tryhamdist;
            keysize = trykeysize;
        }
    }
    printf("keysize = %ld\n", keysize);

    block = new_ba(balen(ba) / keysize);
    key = new_ba(keysize);
    for (i = 0; i < keysize; i++) {
        for (j = 0; j < balen(block) ; j++) {
            block->byte[j] = ba->byte[j * keysize + i];
        }
        findcharxorkey(block, &keychar, &ind);
        key->byte[i] = keychar;
    }

    res = new_ba(balen(ba));
    baxor(res, ba, key);
    //printall(ba);
    printf("Key: \n"); printall(key);
    printf("Result: \n"); printall(res);

    destroy_ba(ba);
    destroy_ba(key);
    destroy_ba(block);
    destroy_ba(res);
    free(buf);
    fclose(stream);
    return 0;
}
