#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>

// 512ビットの「安全素数」を生成する関数
// (p_base が素数、かつ p = 2 * p_base + 1 も素数となる p を探す)
void generate_safe_prime(mpz_ptr p, gmp_randstate_t state) {
    mpz_t p_base;
    mpz_init(p_base);

    while (1) {
        // 1. 511ビットのランダムな奇数を生成（2倍して1足すと512ビットの安全素数になる）
        mpz_urandomb(p_base, state, 511);
        mpz_setbit(p_base, 510); // 511ビット目を1にしてサイズ固定
        mpz_setbit(p_base, 0);   // 奇数にする

        // 2. もとになる数の素数判定
        // GMP 6.3.0では reps=24 以下を指定すると、追加のランダムMiller-Rabinを実行せず、
        // 底2MR + 強Lucas (Baillie-PSW) のみを実行するため最速になります。
        if (mpz_probab_prime_p(p_base, 24) == 0) {
            continue; 
        }

        // 3. p = 2 * p_base + 1 (安全素数) を計算
        mpz_mul_ui(p, p_base, 2);
        mpz_add_ui(p, p, 1);

        // 4. 生成された安全素数 p 自体も素数か判定
        if (mpz_probab_prime_p(p, 24) > 0) {
            break; // 両方素数なら、p は512ビットの「安全素数」として確定
        }
    }

    mpz_clear(p_base);
}

int main() {
    mpz_t p, q, n;
    mpz_inits(p, q, n, NULL);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, (unsigned long)time(NULL));

    printf("GMP 6.3.0 基準: 512bit安全素数からなる 1024bit RSA鍵を生成中...\n\n");

    clock_t start = clock();

    // 安全素数 p と q をそれぞれ生成
    generate_safe_prime(p, state);
    do {
        generate_safe_prime(q, state);
    } while (mpz_cmp(p, q) == 0); // 万が一の重複を防止

    clock_t end = clock();

    // RSA公開鍵 N = p * q
    mpz_mul(n, p, q);

    gmp_printf("p (%zubit 安全素数):\n%Zd\n\n", mpz_sizeinbase(p, 2), p);
    gmp_printf("q (%zubit 安全素数):\n%Zd\n\n", mpz_sizeinbase(q, 2), q);
    gmp_printf("N (%zubit RSA公開鍵):\n%Zd\n\n", mpz_sizeinbase(n, 2), n);
    
    printf("鍵ペア生成時間: %.3f 秒\n", (double)(end - start) / CLOCKS_PER_SEC);

    mpz_clears(p, q, n, NULL);
    gmp_randclear(state);

    return 0;
}

