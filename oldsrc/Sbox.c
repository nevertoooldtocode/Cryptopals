#include <stdio.h>
#include <stdlib.h>

#define ROTL8(x,shift) ((unsigned char) ((x) << (shift)) | ((x) >> (8 - (shift))))

int main() {
    unsigned char sbox[256];
    unsigned char p = 1, q = 1, xformed = 0;
    long i = 0;

    /* loop invariant: p * q == 1 in the Galois field
     * This takes care of the multiplicative inverse */
    do {
        /* multiply p by 3
         * The last term to take care of overflowing most significant bit.
         * 0x1B is the last 8 bits of the reducing polynomial 0x11B */
        p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

        /* divide q by 3 (equals mult by 0xf6. Still have to figure this one out) */
        q ^= q << 1;
        q ^= q << 2;
        q ^= q << 4;
        q ^= q & 0x80 ? 0x09 : 0;

        /* compute the affine transformation */
        xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);
        sbox[p] = xformed ^ 0x63;
    } while (p != 1); // p = 1 means we're back again

    /* 0 is a special case since it has no inverse */
    sbox[0] = 0x63;

    printf("sbox[256] = {");
    for (i = 0; i < 256; i++) {
        printf("0x%02x", sbox[i]);
        if (i != 255) {
            printf(", ");
            if (i % 16 == 15) printf("\n");
        }
    }
    printf("}");
    return 0;
}
