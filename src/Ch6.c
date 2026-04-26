#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ALPHABETSIZE 26
#define WORSTINDICATOR 999999999

typedef struct bitarray {
    long len;
    unsigned char byte[]; // this is the "struct hack", only one malloc needed for the struct
} bitarray;

typedef long histogram[ALPHABETSIZE];
// To store the frequency of the 26 letters of the alphabet in a text

const char HEXALPHABET[16] = "0123456789abcdef";
const char B64ALPHABET[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const histogram ENGFREQ1000 = {81,13,39,39,115,24,18,38,67,1,4,42,35,77,78,24,1,68,67,90,29,6,12,5,13,1};
// Contains average frequency of the 26 letters of the alphabet in an English technical text

void printhisto(histogram his) {
    long i;
    printf("histogram: {");
    for (i = 0; i < ALPHABETSIZE; i++) printf("%ld, ", his[i]);
    printf("}\n");
}

long englishindicator(bitarray *ba) {
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

long balen (bitarray *ba) {
    return ba->len;
}

void printascii(bitarray *ba) {
    long i;
    printf("ascii: ");
    for (i = 0; i < balen(ba); i++) {
        if (ba->byte[i] >= 32 && ba->byte[i] <= 126) printf("%c", ba->byte[i]);
        else printf(" ");
//      else printf("'\\%d'", ba->byte[i]);
    }
    printf("\n");
}

void printhex(bitarray *ba) {
    long i;
    printf("hex: ");
    for (i = 0; i < balen(ba); i++) printf("%02x", ba->byte[i]);
    printf("\n");
}

void print64(bitarray *ba) {
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

void printall(bitarray *ba) {
    printf("length = %ld\n", balen(ba));
    printhex(ba);
    printascii(ba);
    print64(ba);
}

bitarray *new_ba(long bytearraylength) {
    long i;
    bitarray *res = malloc(sizeof(bitarray) + bytearraylength); // Only a single malloc needed because of the "struct hack"
    res->len = bytearraylength;
    for (i = 0; i < res->len; i++) res->byte[i] = 0;
    return res;
}

void destroy_ba(bitarray *ba) {
    free(ba); // Still enough because of the struct hack
}

bitarray *create_ba_from_hex(char *hexstr) {
    // Assumes a clean hexstring, no line feeds
    long i;
    bitarray *res = new_ba(strlen(hexstr) / 2);
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = hexnumvalue(hexstr[2*i]) * 16 + hexnumvalue(hexstr[2*i + 1]);
    }
    return res;
}

bitarray *create_ba_from_64(char *b64str) {
    // Assumes a clean b64string, no line feeds
    long i, leftshift, rightshift;
    bitarray *res = new_ba(((strlen(b64str) - 1) * 3 / 4) + 1);
    for (i = 0; i < balen(res) - 1; i++) {
        leftshift = ((i * 2 + 2) % 6); if (leftshift == 0) leftshift = 6;
        rightshift = (6 - ((i * 2 + 2) % 6)); if (rightshift == 6) rightshift = 0;
        res->byte[i] = (b64numvalue(b64str[i * 4 / 3]) << leftshift) + (b64numvalue(b64str[i * 4 / 3 + 1]) >> rightshift);
    }
    leftshift = ((i * 2 + 2) % 6); if (leftshift == 0) leftshift = 6;
    res->byte[i] = (b64numvalue(b64str[i * 4 / 3]) << leftshift);
    return res;
}

bitarray *create_ba_from_ascii(char *str) {
    long i;
    bitarray *res = new_ba(strlen(str));
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = str[i];
    }
    return res;
}

void update_ba_from_hex(bitarray *ba, char *hexstr) {
    long i;
    for (i = 0; i < balen(ba); i++) {
        ba->byte[i] = hexnumvalue(hexstr[2*i]) * 16 + hexnumvalue(hexstr[2*i + 1]);
    }
}

void update_ba_from_ascii(bitarray *ba, char *str) {
    long i;
    for (i = 0; i < balen(ba); i++) {
        ba->byte[i] = str[i];
    }
}

void bacopy(bitarray *dest, bitarray *src) {
    long i;
    dest->len = src->len;
    for (i = 0; i < balen(src); i++) {
        dest->byte[i] = src->byte[i];
    }
}

int isequalba(bitarray *ba1, bitarray *ba2) {
    long i;
    if (balen(ba1) == balen(ba2)) {
        for (i = 0; i < balen(ba1); i++) {
            if (ba1->byte[i] != ba2-> byte[i]) return 0;
        }
        return 1;
    }
    return 0;
}

long sumbits(char c) {
    long i, res = 0;
    for (i = 0; i < 8; i++) {
        res += (c >> i) & 1;
    }
    return res;
}

long hamming(bitarray *ba1, bitarray *ba2) {
    long i, res = 0;
    for (i = 0; i < balen(ba1); i++) {
        res += sumbits(ba1->byte[i] ^  ba2->byte[i]);
    }
    return res;
}

void xor(bitarray *res, bitarray *ba1, bitarray *ba2) {
    // ba1 should be larger or equal to ba2
    // If ba2 is shorter than ba1, xor is continued with the first byte of ba2
    // res should be created before calling this function, and with length equal to ba1
    long i;
    for (i = 0; i < balen(res); i++) {
        res->byte[i] = ba1->byte[i] ^ ba2->byte[i % ba2->len];
    }
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

void assertall() {
    bitarray *ba1 = create_ba_from_hex("1234abcd");
    bitarray *ba2 = create_ba_from_hex("567890ef");
    bitarray *ba3 = new_ba(balen(ba1));
    bacopy(ba3, ba1);
    bitarray *resxor = new_ba(balen(ba1));
    xor(resxor, ba1, ba2);
    bitarray *resxorcompare = create_ba_from_hex("444c3b22");
    long key = 0, ind = 0;
    findcharxorkey(ba1, &key, &ind);
    //printf("key = %ld, ind = %ld\n", key, ind);

    assert(key == 64 && ind == 415855);
    assert(hexnumvalue('a') == 10);
    assert(b64numvalue('a') == 26);
    assert(balen(ba1) == 4);
    assert(resxor->byte[0] == 0x44);
    assert(isequalba(ba3, ba1));
    assert(isequalba(resxor, resxorcompare));
    assert(isequalba(create_ba_from_hex("ab"), create_ba_from_hex("ab"))); // this works, but is I think a memory leak?
    assert(hamming(create_ba_from_ascii("this is a test"), create_ba_from_ascii("wokka wokka!!!")) == 37);
    //printf("ind: %ld\n", englishindicator(create_ba_from_ascii("hallo world, this is english!")));
    assert(englishindicator(create_ba_from_ascii("hallo world, this is english!")) == 81004);
    destroy_ba(ba1);
    destroy_ba(ba2);
    destroy_ba(resxor);
    destroy_ba(resxorcompare);
}

int main(int argc, char **argv) {

    assertall();

    FILE *stream;
    char *buf = NULL;
    size_t size = 0;
    bitarray *ba = NULL, *res = NULL, *key = NULL, *block = NULL;
    long trykeysize = 0, keysize = 0, tryhamdist = 0, minhamdist = 999999999;
    long i, j, ind, keychar;

    stream = (argc == 1) ? stdin : fopen(argv[1], "rb");

    fseek(stream, 0L, SEEK_END);
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
    xor(res, ba, key);
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
