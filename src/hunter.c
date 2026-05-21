#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <ecm.h>
#include <time.h>

void attack_n(int n_exp) {
    int k_val = 21181;
    mpz_t n, factor;
    mpz_inits(n, factor, NULL);

    // S_n = k * 2^n + 1
    mpz_ui_pow_ui(n, 2, n_exp);
    mpz_mul_ui(n, n, k_val);
    mpz_add_ui(n, n, 1);

    printf("\n--- Targeting n = %d (%lu digits) ---\n", n_exp, mpz_sizeinbase(n, 10));

    ecm_params params;
    ecm_init(params);
    
    // 1. まずは P-1 法で小一時間（あるいは一瞬）チェック
    params->method = ECM_PM1;
    // B1=1e6 程度なら一瞬です
    double b1_p1 = 10000000.0;
    printf("Step 1: P-1 method (B1=%.0f)... ", b1_p1);
    int res = ecm_factor(factor, n, b1_p1, params);
    
    if (res > 0) {
        gmp_printf("\n[!] P-1 Factor found: %Zd\n", factor);
        goto cleanup;
    }
    printf("Not found.\n");

    // 2. ECM 無限ループ
    params->method = ECM_ECM;
    double b1_ecm = 2048000.0; // 最初は小さめで数をこなす
    int curve_count = 0;
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    printf("Step 2: ECM loop (B1=%.0f)...\n", b1_ecm);
    mpz_t sigma;
    mpz_init(sigma);
    while (1) {
        curve_count++;
        
        // ランダムなシード値 sigma を生成
        //mpz_urandomb(sigma, state, 32); 
        mpz_set_ui(params->sigma, 0);

        // ECM実行
        res = ecm_factor(factor, n, b1_ecm, params);
        
        if (curve_count % 10 == 0) {
            printf("\rTested %d curves...", curve_count);
            fflush(stdout);
        }

        if (res > 0) {
            gmp_printf("\n[!] ECM Factor found on curve %d: %Zd\n", curve_count, factor);
            break; 
        }

        // 定期的に B1 を育てる戦略もアリ
        if (curve_count % 500 == 0) {
            b1_ecm *= 2;
            printf("\nIncreasing B1 to %.0f\n", b1_ecm);
        }
        
    }
    mpz_clear(sigma);

cleanup:
    ecm_clear(params);
    gmp_randclear(state);
    mpz_clears(n, factor, NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <n1> <n2> ... or read from a file\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int n_exp = atoi(argv[i]);
        attack_n(n_exp);
    }

    return 0;
}

