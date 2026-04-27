#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitarray.h"

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

unsigned char invsbox(unsigned char sboxchar) {
    long i;
    for (i = 0; i < 256; i++) {
        if (sboxchar == SBOX[i]) break;
    }
    return i;
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
        baxor(resblock, resblock, roundkey[round]);
        //printf("decrypt xor with key: "); printhex(roundkey[round]);
        if (round < 10 ) invmixcolumns(resblock); //Only skipped in the final round
        invshiftrows(resblock);
        invsubbytes(resblock);
    }
    baxor(resblock, resblock, roundkey[0]); // round should be 0 here

}

int main(int argc, char **argv) {

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
    fread(buf, 1,size, stream); // Requires an input file without Linefeeds
    buf[size] = '\0';
    if (buf[size -1] == '\n') buf[size - 1] = '\0'; //get rid of linefeed added by vim
    printf("buf size: %ld\n", strlen(buf));

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