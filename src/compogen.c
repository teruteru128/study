
#include <stdlib.h>
#include <gmp.h>
#include <stdio.h>
#include <sys/random.h>

#define RND_BUFFER_SIZE 2493

int main (int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "%s <素数> <桁数>\n", argv[0]);
        return 1;
    }
    
    // 引数の素数
    mpz_t prime1;
    // 新しく生成した素数
    mpz_t prime2;
    // 出力する合成数
    mpz_t compo;
    // 生成下限
    mpz_t pow_n_min;
    // 生成上限
    mpz_t pow_n_max;
    // 生成ウィンドウ(幅)
    mpz_t window;
    // 生成桁数
    size_t digits = strtoull(argv[2], NULL, 10);
    mpz_inits(prime1, prime2, compo, pow_n_min, pow_n_max, window, NULL);
    mpz_set_str(prime1, argv[1], 10);
    mpz_ui_pow_ui(pow_n_min, 10, digits - 1);
    mpz_ui_pow_ui(pow_n_max, 10, digits);
    mpz_prevprime(pow_n_max, pow_n_max);
    // 乱数
    gmp_randstate_t state;
    gmp_randinit_default(state);
    mpz_t seed;
    mpz_init(seed);
    unsigned char buffer[RND_BUFFER_SIZE];
    getrandom(buffer, RND_BUFFER_SIZE, GRND_NONBLOCK);
    gmp_randclear(state);
    mpz_import(seed, RND_BUFFER_SIZE, 1, 1, 0, 0, buffer);
    gmp_randseed(state, seed);
    mpz_clear(seed);
    mpz_sub(window, pow_n_max, pow_n_min);
    mpz_urandomm(prime2, state, window);
    mpz_add(prime2, prime2, pow_n_min);
    mpz_nextprime(prime2, prime2);
    gmp_printf("素数2: %Zu\n", prime2);
    mpz_mul(compo, prime1, prime2);
    gmp_printf("合成数: %Zu\n", compo);

    mpz_clears(prime1, prime2, compo, pow_n_min, pow_n_max, window, NULL);
    return 0;
}

