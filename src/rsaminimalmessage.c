
#include <stdio.h>
#include <gmp.h>

// 65537乗して209万ビットを超える最小のmsgを求める
int main(int argc, char *argv[]) {
    // 法n
    mpz_t n;
    // 冪乗結果m
    mpz_t m;
    // 暗号化対象メッセージmsg
    mpz_t msg;
    size_t count = 0;
    mpz_inits(n, m, msg, NULL);
    mpz_set_ui(n, 1);
    mpz_mul_2exp(n, n, 1 << 21);
    mpz_set_ui(msg, 1);
    for(;mpz_cmp(n, m) > 0; mpz_mul_2exp(msg, msg, 1), count++) {
        mpz_pow_ui(m, msg, 65537);
    } 
    printf("%zu, %zu bit, %zu回\n", mpz_sizeinbase(m, 2), mpz_sizeinbase(msg, 2), count);
    mpz_clears(n, m, msg, NULL);
    return 0;
}
