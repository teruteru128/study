#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <gmp.h>

int main(int argc, char *argv[]) {
    // 1. 各種変数の初期化
    EVP_PKEY *pkey = NULL;
    BIGNUM *n_bn = NULL;
    mpz_t n_gmp;
    mpz_init(n_gmp);

    // 2. PEMファイルを開く
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("ファイルを開けません");
        return 1;
    }

    // 3. PEMファイルから秘密鍵をEVP_PKEYとして読み込む
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!pkey) {
        fprintf(stderr, "PEMの読み込みに失敗しました。\n");
        mpz_clear(n_gmp);
        return 1;
    }

    // 4. EVP_PKEYからモジュラス "n" をBIGNUMとして抽出する
    if (!EVP_PKEY_get_bn_param(pkey, "n", &n_bn)) {
        fprintf(stderr, "モジュラス(n)の抽出に失敗しました。\n");
        EVP_PKEY_free(pkey);
        mpz_clear(n_gmp);
        return 1;
    }

    // 5. OpenSSLのBIGNUMから16進数文字列を経由してGMPのmpz_tへセット
    char *hex_str = BN_bn2hex(n_bn);
    if (hex_str) {
        // base=16（16進数）として文字列からmpz_tへインポート
        mpz_set_str(n_gmp, hex_str, 16);
        OPENSSL_free(hex_str);
    } else {
        fprintf(stderr, "16進数文字列への変換に失敗しました。\n");
    }

    // 6. 結果の確認（10進数で出力してみる）
    // gmp_printf("n (GMP mpz_t) = %Zd\n", n_gmp);
    printf("hex %zu digits\n", mpz_sizeinbase(n_gmp, 16));
    printf("binary %zu bits\n", mpz_sizeinbase(n_gmp, 2));
    
    mpz_t p, q, r;
    mpz_inits(p, q, r, NULL);
    FILE *fp1 = fopen(argv[2], "r");
    if(fp1 == NULL){
      return 1;
    }
    size_t len = mpz_inp_str(p, fp1, 10);
    printf("prime len: %zu\n", len);
    mpz_add_ui(q, p, 264125 * 2 + 1);
    mpz_add_ui(r, p, 460025 * 2 + 1);
    
    printf("%d\n", mpz_divisible_p(n_gmp, p));
    printf("%d\n", mpz_divisible_p(n_gmp, q));
    printf("%d\n", mpz_divisible_p(n_gmp, r));
    
    mpz_clears(p, q, r, NULL);

    // 7. クリーンアップ
    BN_free(n_bn);
    EVP_PKEY_free(pkey);
    mpz_clear(n_gmp);

    return 0;
}

