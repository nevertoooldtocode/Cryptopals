// include/aes.h

#ifndef AES_H
#define AES_H

#include "bitarray.h"

unsigned char gmul(unsigned char a, unsigned char b); 

void encrypt_aes(bitarray* res, bitarray* ba, bitarray* key);
void decrypt_aes(bitarray* res, bitarray* ba, bitarray* key);
#endif
