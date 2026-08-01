
#include <math.h>
#include <stdio.h>
#include <gmp.h>

int main(int argc, char *argv[]) {
    mpz_t p, n;
    mpz_inits(p, n, NULL);
    mpz_set_ui(n, 1);
    size_t count = 0;
    double sumoflog = 0.;
    while((count = mpz_inp_str(p, stdin, 10)) != 0) {
        mpz_mul(n, n, p);
        if(mpz_fdiv_ui(p, 65537) == 1){
            gmp_fprintf(stderr, "おお %Zu-1 は65537で割れる\n", p);
        }
        sumoflog += log2(mpz_get_ui(p));
    }
    fprintf(stderr, "feof: %d, ferror: %d\n", feof(stdin), ferror(stdin));
    fprintf(stderr, "%zu\n", mpz_sizeinbase(n, 2));
    fprintf(stderr, "%.2f\n", sumoflog);
    mpz_clears(p, n, NULL);
    return 0;
}
