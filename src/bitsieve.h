
#ifndef BITSIEVE_H
#define BITSIEVE_H

#include <stdio.h>
#include <gmp.h>

struct BitSieve;
#ifdef PUBLISH_STRUCT_BS
struct BitSieve
{
    size_t length;
    size_t bits_length;
    unsigned long *bits;
};
#endif
struct BitSieve *bs_getInstance(mpz_t *base, size_t searchLen);
void bs_initInstance(struct BitSieve *bs, mpz_t *base, size_t searchLen);
void bs_free(struct BitSieve *bs);
mpz_t *bs_retrieve(struct BitSieve *bs, mpz_t *initValue, int certainty);
void bs_foreach(struct BitSieve *bs, void (*function)(mpz_t *base, unsigned long offset, void *arg), mpz_t *base, void *arg);

size_t bs_fileout(FILE *stream, const struct BitSieve *bs);
size_t bs_filein(struct BitSieve *bs, FILE *stream);

#endif
