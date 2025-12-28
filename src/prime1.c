
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
    // mpz_set_str(number, "51515155212112155512544451215879794313484631643513515461313510654159642752672634875312153543513515153543513564564984646463564346842311254454685465132211546454881133115554645161516484849874321321348523202152305448405648254896348510549", 10);
    // mpz_set_str(number, "9cfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff3", 16);
    mpz_set_ui(number, 157);
    mpz_mul_2exp(number, number, 504);
    // mpz_sub_ui(number, number, 13);
    // 9cfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff4
    mpz_prevprime(p, number);
    size_t length = mpz_sizeinbase(p, 16) + 2;
    char *buf = malloc(length);
    mpz_get_str(buf, 16, p);
    printf("%s\n", buf);
    free(buf);
    mpz_clears(number, p, NULL);
    return 0;
}
