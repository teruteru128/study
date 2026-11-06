
#include "secure_memzero.h"

// 安全にメモリをゼロクリアする関数
void secure_memzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
}
