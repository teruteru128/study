
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#define MAX_PRIMES 70000

void calculate_crt_params(mpz_t *primes, int count, mpz_t e) {
    mpz_t n, phi, d, tmp, gcd;
    mpz_t d_params[MAX_PRIMES];
    mpz_t inv_params[MAX_PRIMES];
    mpz_t current_mod;

    mpz_inits(n, phi, d, tmp, gcd, current_mod, NULL);
    mpz_set_ui(n, 1);
    mpz_set_ui(phi, 1);

    // 1. モジュラス n と トーシェント phi の計算
    for (int i = 0; i < count; i++) {
        mpz_mul(n, n, primes[i]);
        
        mpz_sub_ui(tmp, primes[i], 1);
        mpz_mul(phi, phi, tmp);
    }

    // 2. 秘密指数 d の計算
    if (mpz_invert(d, e, phi) == 0) {
        fprintf(stderr, "エラー: e と phi が互いに素ではありません。\n");
        goto cleanup;
    }

    // 3. 復号指数 d_i (d mod (p_i - 1)) の計算
    for (int i = 0; i < count; i++) {
        mpz_init(d_params[i]);
        mpz_sub_ui(tmp, primes[i], 1);
        mpz_mod(d_params[i], d, tmp);
    }

    // 4. CRT係数（反転元）の計算
    // 最初の2素数は p と q。q_inv = q^-1 mod p となる (Garnerの定義に合わせる)
    mpz_init(inv_params[0]); // p用（未使用）
    mpz_init(inv_params[1]); // q_inv = q^-1 mod p
    if (mpz_invert(inv_params[1], primes[1], primes[0]) == 0) {
        fprintf(stderr, "エラー: 逆元 q_inv が計算できません。\n");
        goto cleanup;
    }

    // 3つ目以降の素数: r_inv = (p*q)^-1 mod r, s_inv = (p*q*r)^-1 mod s
    mpz_mul(current_mod, primes[0], primes[1]); // 最初は p * q
    for (int i = 2; i < count; i++) {
        mpz_init(inv_params[i]);
        if (mpz_invert(inv_params[i], current_mod, primes[i]) == 0) {
            fprintf(stderr, "エラー: 逆元係数(インデックス %d) が計算できません。\n", i);
            goto cleanup;
        }
        mpz_mul(current_mod, current_mod, primes[i]); // 次のループ用にモジュラスを更新
    }

    // 5. 計算結果の出力 (Hex形式)
    printf("=== Multi-Prime RSA Key Parameters ===\n");
    gmp_printf("Modulus (n): %Zx\n\n", n);
    gmp_printf("Public Exponent (e): %Zx\n\n", e);
    gmp_printf("Private Exponent (d): %Zx\n\n", d);
    
    printf("--- Primes & CRT Exponents ---\n");
    for (int i = 0; i < count; i++) {
        gmp_printf("Prime[%d]: %Zx\n", i + 1, primes[i]);
        gmp_printf("Exponent d_%d: %Zx\n\n", i + 1, d_params[i]);
    }

    printf("--- CRT Coefficients (Inverse) ---\n");
    gmp_printf("q_inv (q^-1 mod p): %Zx\n\n", inv_params[1]);
    for (int i = 2; i < count; i++) {
        gmp_printf("Coefficient[%d] ((p1*...*p%d)^-1 mod p%d): %Zx\n\n", i + 1, i, i + 1, inv_params[i]);
    }

cleanup:
    mpz_clears(n, phi, d, tmp, gcd, current_mod, NULL);
    for (int i = 0; i < count; i++) {
        mpz_clear(d_params[i]);
        mpz_clear(inv_params[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "使い方: %s <素数リストのテキストファイル>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("ファイルを開けませんでした");
        return 1;
    }

    mpz_t primes[MAX_PRIMES];
    int count = 0;

    // ファイルから素数を1行ずつ読み込む
    while (count < MAX_PRIMES) {
        mpz_init(primes[count]);
        if (mpz_inp_str(primes[count], fp, 10) > 0) { // 10進数として読み込み
            count++;
        } else {
            mpz_clear(primes[count]); // 読み込めなかった場合はクリア
            break;
        }
    }
    fclose(fp);

    if (count < 4) {
        fprintf(stderr, "エラー: 多素数RSA鍵には最低4つの素数が必要です（入力数: %d）\n", count);
        for(int i=0; i<count; i++) mpz_clear(primes[i]);
        return 1;
    }

    // 公開指数 e = 65537
    mpz_t e;
    mpz_init_set_ui(e, 65537);

    calculate_crt_params(primes, count, e);

    // メモリ解放
    mpz_clear(e);
    for (int i = 0; i < count; i++) {
        mpz_clear(primes[i]);
    }

    return 0;
}

