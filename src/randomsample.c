
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/random.h>
#include <unistd.h>

// 乱数的な
int random_()
{
    uint64_t a = 0;
    ssize_t ret = getrandom(&a, 6, 0);
    if (ret != 6)
    {
        perror("getrandom");
        return 1;
    }
    a = le64toh(a);
    printf("%lf\n", (16 * (a / (double)(1UL << 48))));
    printf("%lf\n", 539. / 540);
    printf("%lf\n", (1 + sqrt(5)) / 3);
    return 0;
}

// 乱数的な
int random2_()
{
    uint64_t a = 0;
    ssize_t ret = 0;
    uint64_t min = ULONG_MAX;
    do
    {
        ret = getrandom(&a, 6, 0);
        if (ret != 6)
        {
            perror("getrandom");
            return 1;
        }
        if (a == 0)
        {
            min = 0;
            break;
        }
        a = le64toh(a);
        if (a < min)
        {
            min = a;
            printf("%016" PRIx64 "\n", a);
        }
    } while (1);
    return 0;
}

// 乱数的な
int random3_()
{
    unsigned char a[8];
    ssize_t ret = 0;
    unsigned char reduction = 0;
    size_t i = 0;
    while (1)
    {
        ret = getrandom(&a, 8, 0);
        reduction = a[0] | a[1] | a[2] | a[3] | a[4] | a[5] | a[6] | a[7];
        if ((reduction & 0x0f) == 0x00 || (reduction & 0xf0) == 0x00)
        {
            printf("%02x: ", reduction);
            for (i = 0; i < 8; i++)
            {
                printf("%02x", a[i]);
            }
            printf("\n");
        }
    }
    return 0;
}

struct tmp
{
    unsigned long tmp;
    size_t size;
};

double getdouble(struct tmp *tmp)
{
    double out = 0;
    if (tmp->size >= 52)
    {
        out = (double)(((tmp->tmp) >> (52 - tmp->size)) & 0xfffffffffffffUL)
              / (1UL << 52);
        tmp->size -= 52;
        return out;
    }
    unsigned long work = 0;
    if (getrandom(&work, 7, 0) != 7)
    {
        return 1;
    }
    work = le64toh(work);
    tmp->tmp = (tmp->tmp << 4) | ((work >> 52) & 0xf);
    tmp->size += 4;
    out = (double)(work & 0xfffffffffffffUL) / (1UL << 52);
    return out;
}
