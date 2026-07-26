
#include <gmp.h>
#include <stdint.h>
#include <sys/random.h>

int main(int argc, char *argv[]) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    uint64_t seed;
    if(getrandom(&seed, sizeof(seed), GRND_NONBLOCK) < sizeof(seed)){
        return 1;
    }
    gmp_randseed_ui(state, seed);
    mpz_t p;
    mpz_init(p);
    mpz_rrandomb(p, state, 128);
    gmp_printf("%Zx\n", p);
    mpz_clear(p);
    gmp_randclear(state);
    return 0;
}
