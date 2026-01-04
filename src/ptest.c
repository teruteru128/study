
#include <stdio.h>
#include <gmp.h>
#include <time.h>

/**
 * 114514^114514*2^28+1の事前素数性チェック
 */
int main(int argc, char const *argv[])
{
    // mpz_tが内部で配列なの気に食わない
    MP_INT n;
    mpz_init(&n);
    mpz_set_ui(&n, 114514);
    mpz_pow_ui(&n, &n, 114514);
    mpz_mul_2exp(&n, &n, 28);
    mpz_add_ui(&n, &n, 1);
    time_t start = time(NULL);
    int result = mpz_probab_prime_p(&n, 24);
    time_t finish = time(NULL);
    printf("result: %d(%f hours)\n", result, difftime(finish, start) / 3600);
    mpz_clear(&n);
    return 0;
}
