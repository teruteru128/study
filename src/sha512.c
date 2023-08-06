
#include "sha512.h"

void SHA512init(qword *buf)
{
    buf[0] = 0x6a09e667f3bcc908UL;
    buf[1] = 0xbb67ae8584caa73bUL;
    buf[2] = 0x3c6ef372fe94f82bUL;
    buf[3] = 0xa54ff53a5f1d36f1UL;
    buf[4] = 0x510e527fade682d1UL;
    buf[5] = 0x9b05688c2b3e6c1fUL;
    buf[6] = 0x1f83d9abfb41bd6bUL;
    buf[7] = 0x5be0cd19137e2179UL;
}
void SHA512compress(qword *buf, qword *x) {}
void SHA512finish(qword *buf, byte *strptr, qword lswlen, qword mswlen) {}
