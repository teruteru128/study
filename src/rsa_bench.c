
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

// 処理時間を測定するヘルパー関数（ミリ秒単位）
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000000.0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "使い方: %s <RSA秘密鍵のDERファイル>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    
    // 1. DERファイルの読み込み
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("ファイルを開けませんでした");
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    unsigned char *der_buf = malloc(file_size);
    if (fread(der_buf, 1, file_size, fp) != file_size) {
        perror("ファイルの読み込みに失敗しました");
        fclose(fp);
        free(der_buf);
        return 1;
    }
    fclose(fp);

    printf("鍵ファイル: %s (%ld バイト) を読み込み中...\n", filename, file_size);

    // 2. OpenSSL 3.0のAPIでDERから秘密鍵（EVP_PKEY）をデコード
    const unsigned char *p = der_buf;
    EVP_PKEY *pkey = d2i_PrivateKey(EVP_PKEY_RSA, NULL, &p, file_size);
    free(der_buf);

    if (!pkey) {
        fprintf(stderr, "エラー: OpenSSLが秘密鍵のデコードに失敗しました。\n");
        return 1;
    }

    // 鍵情報の表示
    int bits = EVP_PKEY_get_bits(pkey);
    printf("鍵のデコード成功: RSA %d ビット\n", bits);

    // 3. 署名（秘密鍵演算）の準備
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx || EVP_PKEY_sign_init(ctx) <= 0) {
        fprintf(stderr, "エラー: 署名コンテキストの初期化に失敗しました。\n");
        EVP_PKEY_free(pkey);
        return 1;
    }

    // ダミーのデータ（SHA-256ハッシュを想定した32バイト）を用意
    unsigned char digest[32] = {0x01, 0x02, 0x03};
    size_t sig_len = 0;

    // まず署名後のサイズを取得
    if (EVP_PKEY_sign(ctx, NULL, &sig_len, digest, sizeof(digest)) <= 0) {
        fprintf(stderr, "エラー: 署名サイズの取得に失敗しました。\n");
        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return 1;
    }
    unsigned char *sig = malloc(sig_len);

    // 4. ベンチマーク計測（秘密鍵による署名処理）
    printf("秘密鍵演算（署名）を実行中...（時間がかかる場合があります）\n");
    
    double start_time = get_time_ms();
    int ret = EVP_PKEY_sign(ctx, sig, &sig_len, digest, sizeof(digest));
    double end_time = get_time_ms();

    if (ret > 0) {
        printf("【計測結果】 処理時間: %.2f ミリ秒 (約 %.3f 秒)\n", 
               end_time - start_time, (end_time - start_time) / 1000.0);
    } else {
        fprintf(stderr, "エラー: 秘密鍵演算の実行中にエラーが発生しました。\n");
    }

    // 後片付け
    free(sig);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return 0;
}

