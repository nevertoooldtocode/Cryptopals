// tests/test_cryptopals.c

#include <assert.h>
#include "bitarray.h"

void assertall() {
    bitarray *ba1 = create_ba_from_hex("1234abcd");
    bitarray *ba2 = create_ba_from_hex("567890ef");
    bitarray *ba3 = new_ba(balen(ba1));
    bacopy(ba3, ba1);
    bitarray *resxor = new_ba(balen(ba1));
    baxor(resxor, ba1, ba2);
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

int main() {
    assertall();
}
