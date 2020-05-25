
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#define N "135066410865995223349603216278805969938881475605667027524485143851526510604859533833940287150571909441798207282164471551373680419703964191743046496589274256239341020864383202110372958725762358509643110564073501508187510676594629205563685529475213500852879416377328533906109750544334999811150056977236890927563"
#define BASE 10

/*
RSA-1024-challenge
135066410865995223349603216278805969938881475605667027524485143851526510604859533833940287150571909441798207282164471551373680419703964191743046496589274256239341020864383202110372958725762358509643110564073501508187510676594629205563685529475213500852879416377328533906109750544334999811150056977236890927563
*/
#define qfile "q.txt"

int loadP(mpz_t *p)
{
  char buf[1024];
  FILE *fp = NULL;
  int ret = EXIT_SUCCESS;
  char *tmp = NULL;
  if((fp = fopen(qfile, "r")) != NULL)
  {
    tmp = fgets(buf, 1024, fp);
    if(!tmp)
    {
      return EXIT_FAILURE;
    }
    mpz_set_str(*p, buf, 16);
  }
  else
  {
    perror("fopen");
    mpz_set_si(*p, 1);
    ret = EXIT_FAILURE;
  }
  fclose(fp);
  return ret;
}

int main(int argc, char* argv[]){
  mpz_t n;
  mpz_t p;
  mpz_t q;
  mpz_t minQ;
  mpz_t maxQ;
  mpz_t sqQ;
  mpz_t sqrtN;
  mpz_t nSubSqQ;
  mpz_t nSubSqQModQ;
  mpz_t doubledSqQ;
  mpz_t pSubQ;
  mpz_t num;
  mpz_inits(n, p, q, minQ, maxQ, sqQ, sqrtN, nSubSqQ, nSubSqQModQ, doubledSqQ, pSubQ, num, NULL);
  mpz_set_str(n, N, BASE);
  loadP(&q);
  // qを1からsqrt(n)までループ, (n-q^2)/qがqより大きくなるまでループ
  int i=0;
  //mpz_set_str(q, "b0000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002", 16);
  mpz_sqrt(sqrtN, n);
  mpz_mul(sqQ, q, q);
  mpz_mul_si(doubledSqQ, sqQ, 2);
  mpz_sub(nSubSqQ, n, sqQ);
  mpz_fdiv_qr(pSubQ, nSubSqQModQ, nSubSqQ, q);
  mpz_add(p, pSubQ, q);
  printf("OK\n");
  printf("OK!\n");
  gmp_printf("n =              %Zd\n", n);
  gmp_printf("n =              %Zx\n", n);
  gmp_printf("2q^2 =           %Zd\n", doubledSqQ);
  gmp_printf("q =              %155Zd\n", q);
  gmp_printf("sqrt(n) =        %155Zd\n", sqrtN);
  gmp_printf("n - q*q =        %Zd\n", nSubSqQ);
  gmp_printf("p - q =          %155Zd\n", pSubQ);
  gmp_printf("(n - q*q) %% q = %156Zd\n", nSubSqQModQ);
}

