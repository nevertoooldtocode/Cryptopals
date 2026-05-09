#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitarray.h"
#include "aes.h"



int main(int argc, char **argv) {

    FILE *stream;
    char *buf = NULL;
    size_t size = 0;
    bitarray *ba = NULL, *res = NULL, *key = NULL;

    stream = (argc == 1) ? stdin : fopen(argv[1], "rb");

    fseek(stream, 0L, SEEK_END);
    size = ftell(stream);
    buf = malloc(size+1);
    rewind(stream);
    fread(buf, 1,size, stream); // Requires an input file without Linefeeds
    buf[size] = '\0';
    if (buf[size -1] == '\n') buf[size - 1] = '\0'; //get rid of linefeed added by vim
    ba = create_ba_from_64(buf);
    free(buf);
    fclose(stream);


    key = create_ba_from_ascii("YELLOW SUBMARINE");
    res = new_ba(balen(ba));

    decrypt_aes_ecb(res, ba, key);

    printf("Result: \n"); printascii(res);

    destroy_ba(ba);
    destroy_ba(key);
    destroy_ba(res);
    return 0;
}