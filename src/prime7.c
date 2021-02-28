
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "bitsieve.h"

#include <gmp.h>

#define BIT_LENGTH 262144
#define SEARCH_LENGTH (BIT_LENGTH / 20 * 64)
struct BitSieve
{
    unsigned long *bits;
    size_t bits_length;
    size_t length;
};

static int threadpool_live = 1;

/*
スレッドプール終了変数を初期化する
タスクキューと完了済みタスクキューを生成する
スレッドプールを生成する
while(1)
{
タスクを生成してタスクキューに追加するoffset [0, 838860)
タスクが全て完了するのを待つ
}
*/
int main(int argc, char *argv[])
{
    mpz_t initValue;
    mpz_init(initValue);
    FILE *fin = fopen("262144bit-initialValue2.txt", "r");
    if (fin == NULL)
    {
        mpz_clear(initValue);
        perror("fopen");
        return EXIT_FAILURE;
    }
    mpz_inp_str(initValue, fin, 16);
    fclose(fin);
    struct BitSieve searchSieve;
    bs_initInstance(&searchSieve, &initValue, (size_t)SEARCH_LENGTH);
    size_t k = 0;
    for (size_t i = 0; i < searchSieve.bits_length; i++)
    {
        unsigned long nextLong = ~searchSieve.bits[i];
        //printf("%016lx\n", nextLong);
        for (size_t j = 0; j < 64; j++)
        {
            if ((nextLong & 1) == 1)
            {
                printf("%zu : +%lu\n", k++, (i * 64 + j) * 2 + 1);
            }
            nextLong >>= 1;
        }
    }
    bs_free(&searchSieve);
    mpz_clear(initValue);
    return EXIT_SUCCESS;
}
