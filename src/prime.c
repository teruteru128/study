
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>

/*
 * 素数判定
 * 素数探索
 *   階乗素数
 *   ハミング重みが3の素数(x^2 + y^2 + 1, x > y > 1)
 *   x^2 + y^2 - 1, x > y > 1
 *   x^2 - y^2 + 1, x > y > 1
 *   x^2 - y^2 - 1, x > y > 1
 */
int main(int argc, char *argv[])
{
  mpz_t number;
  mpz_t p;
  int answer = 0;
  mpz_inits(number, p, NULL);
  //mpz_set_str(number, "51515155212112155512544451215879794313484631643513515461313510654159642752672634875312153543513515153543513564564984646463564346842311254454685465132211546454881133115554645161516484849874321321348523202152305448405648254896348510549", 10);
  mpz_set_str(number, "9cfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff3", 16);
  mpz_set(p, number);
  //9cfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff4
  while (1)
  {
    answer = mpz_probab_prime_p(p, 25);
    if (answer == 1 || answer == 2)
    {
      mpz_out_str(stdout, 16, p);
      fputs("\n", stdout);
      break;
    }
    else
    {
      fputs("no\n", stdout);
    }
    mpz_sub_ui(p, p, 1);
  }
  mpz_clears(number, p, NULL);
  return 0;
}
