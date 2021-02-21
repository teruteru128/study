
#ifndef BITSIEVE_H
#define BITSIEVE_H

#include <stdio.h>
#include <gmp.h>

struct BitSieve;
struct BitSieve *bs_getInstance(mpz_t *base, size_t searchLen);
void bs_initInstance(struct BitSieve *bs, mpz_t *base, size_t searchLen);
void bs_free(struct BitSieve *bs);
mpz_t *bs_retrieve(struct BitSieve *bs, mpz_t *initValue, int certainty);

#endif
