// src/Ch7.c
//
// This is Challenge 7 of Set 1 of Cryptopals
// Decode a file with AES-128 in ECB mode with a known key

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitarray.h"
#include "aes.h"

char* string_from_file(char* filename) {
    char *buf = NULL;
    size_t size = 0;
    int c = 0, i = 0;
    FILE* fp = fopen(filename, "rb");
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    buf = malloc(size + 1);
    rewind(fp);
    while ((c = fgetc(fp)) != EOF) {
	if (c != '\n' && c != '\r') 
	    buf[i++] = (char) c;
    }
    buf[i] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    char* buf = NULL;
    bitarray *ba = NULL, *res = NULL, *key = NULL;

    if (argc != 2) {
	fprintf(stderr, "usage: %s <filename>\n", argv[0]);
	exit(1);
    }
    buf = string_from_file(argv[1]);
    ba = create_ba_from_64(buf);
    free(buf);

    key = create_ba_from_ascii("YELLOW SUBMARINE");
    res = new_ba(balen(ba));

    decrypt_aes_ecb(res, ba, key);

    printf("Result: \n"); printascii(res);

    destroy_ba(ba);
    destroy_ba(key);
    destroy_ba(res);
    return 0;
}