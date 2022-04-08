
#ifndef BITSIEVE_H
#define BITSIEVE_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <gmp.h>
#include "bitset.h"

typedef struct BitSieve BitSieve;
#ifdef PUBLISH_STRUCT_BS
struct BitSieve
{
    size_t length;
    size_t bits_length;
    uint64_t *bits;
};
#endif

struct bs_ctx
{
    size_t start;
    pthread_mutex_t mutex;
};
struct BitSieve *bs_getInstance(mpz_t *base, size_t searchLen);
void bs_initInstance(struct BitSieve *bs, mpz_t *base, size_t searchLen);
void bs_free(struct BitSieve *bs);
mpz_t *bs_retrieve(struct BitSieve *bs, mpz_t *initValue, int certainty);
void bs_foreach(struct BitSieve *bs, void (*function)(mpz_t *base, unsigned long offset, void *arg), mpz_t *base, void *arg);

size_t bs_fileout(FILE *stream, const struct BitSieve *bs);
size_t bs_filein(struct BitSieve *bs, FILE *stream);

void bs_initSmallSieve();

size_t bs_getNextStep(struct bs_ctx *ctx);

#endif
