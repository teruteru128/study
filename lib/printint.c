
#include <stdio.h>
#include <stdint.h>

#define DIGITS ("0123456789abcfef")
#define DigitTens ("0000000000111111111122222222223333333333444444444455555555556666666666777777777788888888889999999999")
#define DigitOnes ("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789")

/**
 * from JDK1.8.0_162 Long.stringSize(long)
 */
static size_t stringSize(const uint64_t l)
{
    size_t i;
    uint64_t p = 10;
    for (i = 1; i < 20; i++)
    {
        if (l < p)
        {
            return i;
        }
        p = 10 * p;
    }
    return 20;
    //return stringSizeRec(l, 1, 10);
}

/**
 * from JDK1.8.0_162 Long.toUnsignedString(long), Long.toString(long), Long.getChars(long, int, char[])
 */
size_t snprintUInt64(char *restrict s, size_t n, uint64_t l)
{
    uint64_t tmp;
    size_t length;
    /* length = min(length, n); */
    //length = length < n ? length : n;
    uint64_t q;
    uint32_t r;
    size_t charPos;
    size_t i;

    if (n == 0)
    {
        return 0;
    }

    tmp = l;
    length = stringSize(tmp);
    for (i = length; i > n; i--)
    {
        tmp = tmp / 10;
    }
    charPos = i;

    while (tmp > 0x7FFFFFFFULL)
    {
        q = tmp / 100;
        r = (uint32_t)(tmp - ((q << 6) + (q << 5) + (q << 2)));
        tmp = q;
        s[--charPos] = DigitOnes[r];
        s[--charPos] = DigitTens[r];
    }
    uint32_t q2;
    uint32_t i2 = (uint32_t)tmp;
    while (i2 >= 65536)
    {
        q2 = i2 / 100;
        r = (i2 - ((q2 << 6) + (q2 << 5) + (q2 << 2)));
        i2 = q2;
        s[--charPos] = DigitOnes[r];
        s[--charPos] = DigitTens[r];
    }

    for (;;)
    {
        q2 = (i2 * 52429) >> (16 + 3);
        r = i2 - ((q2 << 3) + (q2 << 1));
        s[--charPos] = DIGITS[r];
        i2 = q2;
        if (i2 == 0)
        {
            break;
        }
    }
    return i;
}

size_t snprintInt64(char *restrict s, size_t n, int64_t l)
{
    return 0;
}
