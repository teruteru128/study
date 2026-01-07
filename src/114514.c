
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define MAX_LENGTH 114514

void calculate_expression(mpz_t result, unsigned long n)
{
    mpz_t term1, base, power_n;

    // 変数の初期化
    mpz_init(term1);
    mpz_init(base);
    mpz_init(power_n);

    // 1. (2^6 * 25^3)^n = (64 * 15625)^n = 1000000^n を計算
    // 式の整理： 2^(6n+5) * 25^(3n+2) = 2^5 * 25^2 * (2^6 * 25^3)^n
    //                               = 32 * 625 * 1000000^n = 20000 * 1000000^n
    mpz_ui_pow_ui(power_n, 1000000, n);

    // 2. 57257 * 20000 * 1000000^n を計算
    // 57257 * 20000 = 1145140000
    mpz_mul_ui(term1, power_n, 1145140000UL);

    // 3. 分子の計算: term1 - 1144140001
    mpz_sub_ui(term1, term1, 1144140001UL);

    // 4. 999999 で割る
    mpz_tdiv_q_ui(result, term1, 999999UL);

    // メモリ解放
    mpz_clear(term1);
    mpz_clear(base);
    mpz_clear(power_n);
}

static size_t getlength(MP_INT *n)
{
    char n_str[229038];
    mpz_get_str(n_str, 10, n);
    return strlen(n_str);
}

/**
 * 114514桁の素数を探索するための事前ツール
 */
int main(int argc, char const *argv[])
{
    mpz_t n, result;
    mpz_init(result);

    // 例として n = 2 を計算
    unsigned long n_val = 2;
    calculate_expression(result, n_val);

    // 結果の出力
    gmp_printf("n = %lu のとき、結果は: %Zd\n", n_val, result);

    mpz_clear(result);
    return 0;
}
