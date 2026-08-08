
#include <gmp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    mpz_t n, p, gcd;
    mpz_inits(n, p, gcd, NULL);
    mpz_set_ui(n, 1);
    while(mpz_inp_str(p, stdin, 10) > 0) {
        mpz_gcd(gcd, n, p);
        if(mpz_cmp_ui(gcd, 1) > 0) {
            gmp_printf("%Zu, %Zu\n", gcd, p);
            mpz_remove(n, n, gcd);
        }
        mpz_mul(n, n, p);
    }
    size_t bitlength = mpz_sizeinbase(n, 2);
    fprintf(stderr, "%zu bits, %f KiB\n", bitlength, bitlength / 8192.);
    mpz_clears(n, p, gcd, NULL);
    return 0;
}
