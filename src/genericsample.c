
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define call_sub(n) _Generic((n), int:sub_int, long: sub_long, default:sub_any)(n)

void sub_int(int val)
{
    printf("%s %d\n", __func__, val);
}

void sub_long(long val)
{
    printf("%s %ld\n", __func__, val);
}

void sub_any(char val)
{
    printf("%s %d\n", __func__, val);
}

/**
 * @brief _Generic文のサンプル
 * 
 * @see https://twitter.com/yutamonsan/status/1391327509627629573
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, const char *argv[])
{
    int n = 0;
    long ln = 2;
    char cn = 5;

    call_sub(n);
    call_sub(ln);
    call_sub(cn);

    return EXIT_SUCCESS;
}
