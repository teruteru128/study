
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rsa1024.h"
#define SQRT_RSA_1024                                                          \
  ("61010335669222"                                                            \
   "10362775848843"                                                            \
   "04342266592405"                                                            \
   "29938912424519"                                                            \
   "32861862025497"                                                            \
   "93063658618503"                                                            \
   "39108937034593"                                                            \
   "69951328434771"                                                            \
   "91458334610516"                                                            \
   "14689649154484"                                                            \
   "55746310700313")
#define BASE 16
#define T_NUM 16

/*
 * 135066410865995223349603216278805969938881475605667027524485143851526510604
 * 859533833940287150571909441798207282164471551373680419703964191743046496589
 * 274256239341020864383202110372958725762358509643110564073501508187510676594
 * 629205563685529475213500852879416377328533906109750544334999811150056977236
 * 890927563
 * RSA-1024-challenge
 */
int main(int argc, char *argv[]) {
  mpz_t n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ,
      pSubQ, num;
  mpz_inits(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ,
            doubledSqQ, pSubQ, num, NULL);
  mpz_set_str(n, N, BASE);
  if(argc >= 2 && strcmp(argv[1], "--bottom-up") == 0)
  {
      mpz_mul_2exp(n, n, 3);
      mpz_add_ui(n, n, 7);
  }
  gmp_printf("%Zd\n", n);
  while(mpz_cmp_ui(n, 1) > 0)
  {
      if(mpz_even_p(n))
      {
          mpz_fdiv_q_2exp(n, n, 1);
      }
      else {
          mpz_mul_ui(n, n, 3);
          mpz_add_ui(n, n, 1);
      }
      gmp_printf("%Zd\n", n);
  }
  mpz_clears(n, p, q, r, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ,
             doubledSqQ, pSubQ, num, NULL);
  return 0;
}
