// include/bitarray.h

#ifndef BITARRAY_H
#define BITARRAY_H

extern const char HEXALPHABET[16];
extern const char B64ALPHABET[64];

typedef struct bitarray {
    long len;
    unsigned char byte[]; // this is the "struct hack", only one malloc needed for the struct
} bitarray;

long balen (bitarray* ba); 

void printascii(bitarray* ba); 

void printhex(bitarray* ba); 

void print64(bitarray* ba); 

void printall(bitarray* ba); 

bitarray* new_ba(long bytearraylength); 

void destroy_ba(bitarray* ba); 

unsigned char hexnumvalue(char hexchar); 

unsigned char b64numvalue(char b64char); 

bitarray* create_ba_from_hex(char* hexstr); 

bitarray* create_ba_from_64(char *b64str); 

bitarray* create_ba_from_ascii(char*str); 

void update_ba_from_hex(bitarray* ba, char* hexstr); 

void update_ba_from_ascii(bitarray* ba, char* str); 

void bacopy(bitarray* dest, bitarray* src); 

int isequalba(bitarray* ba1, bitarray* ba2); 

void baxor(bitarray *res, bitarray *ba1, bitarray *ba2);
#endif