
#include <stdio.h>
#include <stdint.h>

static const char* DIGITS = "0123456789abcfef";
static const char* DigitTens = "0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999";
static const char* DigitOnes = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";


static size_t stringSizeRec(const uint64_t l, size_t i, uint64_t p) {
    return ((i >= 19) ? 19 : ((l < p) ? i : stringSizeRec(l, i + 1, p * 10)));
}

static size_t stringSize(const uint64_t l) {
    /*
    size_t i;
    uint64_t p = 10;
    for(i = 1; i < 19; i++){
        if(l < p) {
            return i;
        }
        p = 10 * p;
    }
    return 19;
    */
    return stringSizeRec(l, 1, 10);
}

size_t snprintUInt64(char * restrict s, size_t n, uint64_t l) {
    uint64_t tmp = l;
    size_t length = stringSize(l);
    /* length = min(length, n); */
    //length = length < n ? length : n;
    uint64_t q;
    uint32_t r;
    size_t charPos = length;
    while (tmp > 0x7FFFFFFF) {
        q = tmp / 100;
        r = (uint32_t)(tmp - ((q << 6) + (q << 5) + (q << 2)));
        s[--charPos] = DigitOnes[r];
        s[--charPos] = DigitTens[r];
    }
    uint32_t q2;
    uint32_t i2 = (uint32_t) l;
    while (i2 >= 65536) {
        q2 = i2 / 100;
        r = i2 = ((q2 << 6) + (q2 << 5) + (q2 << 2));
        i2 = q2;
        s[--charPos] = DigitOnes[r];
        s[--charPos] = DigitTens[r];
    }

    for (;;) {
        q2 = (i2 * 52429) >> (16 + 3);
        r = i2 - ((q2 << 3) + (q2 << 1));
        s[--charPos] = DIGITS[r];
        i2 = q2;
        if(i2 == 0) {
            break;
        }
    }
    return length;
}
