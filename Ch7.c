#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ALPHABETSIZE 26
#define WORSTINDICATOR 999999999
#define ROTL8(x,shift) ((unsigned char) ((x) << (shift)) | ((x) >> (8 - (shift))))

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

const unsigned char SBOX[256] = {  // Rijndael S-box
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

const unsigned char RCON[11] = {0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
//AES Key schedule Round Constants for rounds 1-10. RCON[0] is a dummy.

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

unsigned char gmul(unsigned char a, unsigned char b) {
    // Galois Field (256) multiplication of 2 bytes
    unsigned char res = 0;
    long i, hi_bit_set;
    for (i = 0; i < 8; i++) {
        if ((b & 1) != 0) res ^= a;
        hi_bit_set = ((a & 0x80) != 0);
        a <<= 1;
        if (hi_bit_set) a ^= 0x1b; // x^8 + x^4 + x^3 + x + 1, but minus degree 8 because 8 bits only, so 0x1b instead of 0x11b
        b >>= 1;
    }
    return res;
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

unsigned char invsbox(unsigned char sboxchar) {
    long i;
    for (i = 0; i < 256; i++) {
        if (sboxchar == SBOX[i]) break;
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
    rightshift = (6 - ((i * 2 + 2) % 6)); if (rightshift == 6) rightshift = 0;
    res->byte[i] = (b64numvalue(b64str[i * 4 / 3]) << leftshift) + (b64numvalue(b64str[i * 4 / 3 + 1]) >> rightshift);
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

    assert(gmul(0xff, 9) == 0x46);
    assert(gmul(0xff, 11) == 0xa3);
    assert(gmul(0xff, 13) == 0x97);
    assert(gmul(13, 0xff) == 0x97);
    assert(gmul(0xff, 14) == 0x8d);
    assert(invsbox(0x84) == 0x4f);
    assert(ROTL8(0x80,1) == 1);
    assert(key == 64 && ind == 415855);
    assert(hexnumvalue('a') == 10);
    assert(b64numvalue('a') == 26);
    assert(balen(ba1) == 4);
    assert(resxor->byte[0] == 0x44);
    assert(isequalba(ba3, ba1));
    assert(isequalba(resxor, resxorcompare));
    assert(isequalba(create_ba_from_hex("ab"), create_ba_from_hex("ab"))); // this works, but is I think a memory leak?
    assert(hamming(create_ba_from_ascii("this is a test"), create_ba_from_ascii("wokka wokka!!!")) == 37);
    assert(englishindicator(create_ba_from_ascii("hallo world, this is english!")) == 81004);
    destroy_ba(ba1);
    destroy_ba(ba2);
    destroy_ba(resxor);
    destroy_ba(resxorcompare);
}

void extract_block(bitarray *block, bitarray *ba, long offset){
    long i;
    for (i = 0; i < balen(block); i++) {
        block->byte[i] = ba->byte[i + offset];
    }
}

void insert_block(bitarray *res, bitarray *block, long offset) {
    long i;
    for (i = 0; i < balen(block); i++) {
        res->byte[i + offset] = block->byte[i];
    }
}
void generate_roundkey(bitarray **rk, bitarray *key) {
    // Length of both *key and **rk is 16
    long i, r;
    bacopy(rk[0], key);
    printf("round %d, roundkey: ", 0); printhex(rk[0]);
    for (r = 1; r < 11; r++) {
        rk[r]->byte[0] = rk[r - 1]->byte[0] ^ SBOX[rk[r - 1]->byte[13]] ^ RCON[r];
        rk[r]->byte[1] = rk[r - 1]->byte[1] ^ SBOX[rk[r - 1]->byte[14]];
        rk[r]->byte[2] = rk[r - 1]->byte[2] ^ SBOX[rk[r - 1]->byte[15]];
        rk[r]->byte[3] = rk[r - 1]->byte[3] ^ SBOX[rk[r - 1]->byte[12]];
        for (i = 4; i < 16; i++) {
            rk[r]->byte[i] = rk[r - 1]->byte[i] ^ rk[r]->byte[i - 4];
        }
        printf("round %ld, roundkey: ", r); printhex(rk[r]);
    }
}

void invmixcolumns(bitarray *b) {
    // Expecting a 16 byte block
    long i, j;
    unsigned char rescolumn[4];
    for (i = 0; i < 16; i += 4) {
        rescolumn[0] = gmul(14, b->byte[0 + i]) ^ gmul(11, b->byte[1 + i]) ^ gmul(13, b->byte[2 + i]) ^ gmul(9, b->byte[3 + i]);
        rescolumn[1] = gmul(9, b->byte[0 + i]) ^ gmul(14, b->byte[1 + i]) ^ gmul(11, b->byte[2 + i]) ^ gmul(13, b->byte[3 + i]);
        rescolumn[2] = gmul(13, b->byte[0 + i]) ^ gmul(9, b->byte[1 + i]) ^ gmul(14, b->byte[2 + i]) ^ gmul(11, b->byte[3 + i]);
        rescolumn[3] = gmul(11, b->byte[0 + i]) ^ gmul(13, b->byte[1 + i]) ^ gmul(9, b->byte[2 + i]) ^ gmul(14, b->byte[3 + i]);
        for (j = 0; j < 4; j++) b->byte[j + i] = rescolumn[j];
    }
}

void invshiftrows(bitarray *b) {
    unsigned char temp;
    temp = b->byte[13];
    b->byte[13] = b->byte[9];
    b->byte[9] = b->byte[5];
    b->byte[5] = b->byte[1];
    b->byte[1] = temp;

    temp = b->byte[2];
    b->byte[2] = b->byte[10];
    b->byte[10] = temp;
    temp = b->byte[6];
    b->byte[6] = b->byte[14];
    b->byte[14] = temp;

    temp = b->byte[3];
    b->byte[3] = b->byte[7];
    b->byte[7] = b->byte[11];
    b->byte[11] = b->byte[15];
    b->byte[15] = temp;
}

void invsubbytes(bitarray *block) {
    long i;
    for (i = 0; i < balen(block); i++) {
        block->byte[i] = invsbox(block->byte[i]);
    }
}

void decrypt_aes_block(bitarray *resblock, bitarray *block, bitarray **roundkey) {
    long round;
    bacopy(resblock, block);
    for (round = 10; round > 0; round--) { // decrypt, dus we tellen terug
        xor(resblock, resblock, roundkey[round]);
        //printf("decrypt xor with key: "); printhex(roundkey[round]);
        if (round < 10 ) invmixcolumns(resblock); //Only skipped in the final round
        invshiftrows(resblock);
        invsubbytes(resblock);
    }
    xor(resblock, resblock, roundkey[0]); // round should be 0 here

}

int main(int argc, char **argv) {

    assertall();

    FILE *stream;
    char *buf = NULL;
    size_t size = 0;
    bitarray *ba = NULL, *res = NULL, *key = NULL, *block = NULL, *resblock = NULL;
    //bitarray *testblock = NULL;
    long i = 0, offset = 0, round = 0;

    stream = (argc == 1) ? stdin : fopen(argv[1], "rb");

    fseek(stream, 0L, SEEK_END);
    size = ftell(stream);
    buf = malloc(size+1);
    rewind(stream);
    fread(buf, 1,size, stream);
    buf[size] = '\0';
    if (buf[size -1] == '\n') buf[size - 1] = '\0'; //get rid of linefeed added by vim
    printf("buf size: %d\n", strlen(buf));

    ba = create_ba_from_64(buf);
    printall(ba);
    free(buf);
    fclose(stream);


    key = create_ba_from_ascii("YELLOW SUBMARINE");
    //testblock = create_ba_from_hex("5de070bb9fdc589d010101014d7ebdf8");
    block = new_ba(balen(key));
    resblock = new_ba(balen(block));
    res = new_ba(balen(ba));

    bitarray *roundkey[11]; // Initialize roundkey array
    for (round = 0; round < 11; round++) {
        roundkey[round] = new_ba(balen(key));
    }
/*
    printf("testblock: "); printhex(testblock);
    invmixcolumns(testblock);
    printf("mixed columns testblock: "); printhex(testblock);
    invshiftrows(testblock);
    printf("shifted rows testblock: "); printhex(testblock);
*/
    generate_roundkey(roundkey, key);

    for (i = 0; i < (balen(ba) / balen(block)); i++){
        offset = i * balen(block);
        extract_block(block, ba, offset);
        decrypt_aes_block(resblock, block, roundkey);
        insert_block(res, resblock, offset);
    //printf("Result after block %ld: ", i); printascii(res);
    }

    printf("Result: \n"); printascii(res);

    for (round = 0; round < 11; round++) {
        destroy_ba(roundkey[round]);
    }
    destroy_ba(ba);
    destroy_ba(key);
    destroy_ba(block);
    destroy_ba(resblock);
    destroy_ba(res);
    return 0;
}