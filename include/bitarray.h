// include/bitarray.h
//
// Data structure bitarray, a simple array of bytes + length.
// Plus supporting functions

#ifndef BITARRAY_H
#define BITARRAY_H

extern const char HEXALPHABET[16];
extern const char B64ALPHABET[64];

typedef struct bitarray {
    long len;
    unsigned char byte[]; // this is the "struct hack", only one malloc needed for the struct
} bitarray;

// Create and destroy
bitarray* new_ba(long bytearraylength); 
bitarray* create_ba_from_hex(char* hexstr); 
bitarray* create_ba_from_64(char *b64str); 
bitarray* create_ba_from_ascii(char*str); 
void destroy_ba(bitarray* ba); 

//Update
void update_ba_from_hex(bitarray* ba, char* hexstr); 
void update_ba_from_ascii(bitarray* ba, char* str); 

//Copy
void bacopy(bitarray* dest, bitarray* src); 

//Is Equal
int isequalba(bitarray* ba1, bitarray* ba2); 

//XOR
void baxor(bitarray *res, bitarray *ba1, bitarray *ba2);

//Array length
long balen (bitarray* ba); 

// Printing functions
void printascii(bitarray* ba); 
void printhex(bitarray* ba); 
void print64(bitarray* ba); 
void printall(bitarray* ba); 

#endif