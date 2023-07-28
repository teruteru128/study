
#include "../src/ripemd160.h"
#include "../src/rmd160.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    unsigned char input[64] = { 0 };
    unsigned char hash[20];
    ripemd160(hash, input);
    int ret = 0;
    unsigned char v[20]
        = { 0x9b, 0x8c, 0xcc, 0x2f, 0x37, 0x4a, 0xe3, 0x13, 0xa9, 0x14,
            0x76, 0x3c, 0xc9, 0xcd, 0xfb, 0x47, 0xbf, 0xe1, 0xc2, 0x29 };
    ret |= memcmp(hash, v, 20);
    for (int i = 0; i < 20; i++)
        printf("%02x", hash[i]);
    printf("\n");
    return ret != 0;
}
