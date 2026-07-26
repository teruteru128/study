
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <gmp.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define POOL_FILE "entropy_pool.bin"
#define POOL_SIZE 64      // 512ビット (64バイト)
#define OUTPUT_SIZE 32    // 256ビット (32バイト)

// 安全にメモリをゼロクリアする関数
void secure_memzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
}

// 16進数文字列を表示する補助関数
void print_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

// プールファイルの読み込み（存在しない場合はOSの乱数で初期化）
int load_pool(uint8_t *pool) {
    FILE *f = fopen(POOL_FILE, "rb");
    if (f) {
        size_t read_bytes = fread(pool, 1, POOL_SIZE, f);
        fclose(f);
        if (read_bytes == POOL_SIZE) return 0;
    }

    // 初回：OSの安全な乱数で初期化
    if (RAND_bytes(pool, POOL_SIZE) != 1) {
        fprintf(stderr, "エラー: OS乱数での初期化に失敗しました。\n");
        return -1;
    }
    
    // 保存
    f = fopen(POOL_FILE, "wb");
    if (!f) return -1;
    fwrite(pool, 1, POOL_SIZE, f);
    fclose(f);
    return 0;
}

// プールファイルの保存
int save_pool(const uint8_t *pool) {
    FILE *f = fopen(POOL_FILE, "wb");
    if (!f) return -1;
    fwrite(pool, 1, POOL_SIZE, f);
    fclose(f);
    return 0;
}

// サイコロの目を6進数としてGMPで計算し、プールに撹拌する
int feed_dice(uint8_t *pool, const char *dice_str) {
    size_t len = strlen(dice_str);
    size_t valid_count = 0;

    // GMPの多倍長整数型を初期化
    mpz_t big_int;
    mpz_init_set_ui(big_int, 0);

    // 1〜6の文字を処理し、6進数として数値を組み立てる
    for (size_t i = 0; i < len; i++) {
        if (dice_str[i] >= '1' && dice_str[i] <= '6') {
            uint32_t digit = dice_str[i] - '1'; // 0〜5に変換
            mpz_mul_ui(big_int, big_int, 6);    // 現在の値を6倍
            mpz_add_ui(big_int, big_int, digit); // 今回の目を足す
            valid_count++;
        }
    }

    if (valid_count == 0) {
        mpz_clear(big_int);
        return 0;
    }

    // GMPの整数からビッグエンディアンのバイト列を抽出
    size_t countp = 0;
    // mpz_exportはメモリを内部でmallocしてバイト列を出力してくれる
    uint8_t *dice_bytes = (uint8_t *)mpz_export(NULL, &countp, 1, 1, 1, 0, big_int);
    
    // 万が一、出目がすべて1（値が0）だった場合はmpz_exportがNULLを返すためのハンドリング
    if (dice_bytes == NULL) {
        dice_bytes = calloc(1, 1);
        countp = 1;
    }

    // 【撹拌】OpenSSL EVP を使用した SHA-512 計算
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned int md_len;
    
    EVP_DigestInit_ex(ctx, EVP_sha512(), NULL);
    EVP_DigestUpdate(ctx, pool, POOL_SIZE);
    EVP_DigestUpdate(ctx, dice_bytes, countp); // GMPから切り出した正確なバイト長
    EVP_DigestFinal_ex(ctx, pool, &md_len);     // poolを新しい状態で上書き
    EVP_MD_CTX_free(ctx);

    // 一時メモリの安全な消去と解放
    secure_memzero(dice_bytes, countp);
    free(dice_bytes);
    
    // GMPのメモリ解放
    mpz_clear(big_int);

    save_pool(pool);
    return (int)valid_count;
}

// プールから256ビットの鍵を切り出し、プールを自己更新する
int extract_key(uint8_t *pool, uint8_t *output_key) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned int md_len;

    // 1. 256ビットの鍵を抽出 (SHA-256を使用)
    const char *key_salt = "EXTRACT_KEY";
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, pool, POOL_SIZE);
    EVP_DigestUpdate(ctx, key_salt, strlen(key_salt));
    EVP_DigestFinal_ex(ctx, output_key, &md_len);

    // 2. 前方向セキュリティのためプール(512ビット)を自己更新
    const char *state_salt = "NEXT_STATE";
    EVP_DigestInit_ex(ctx, EVP_sha512(), NULL);
    EVP_DigestUpdate(ctx, pool, POOL_SIZE);
    EVP_DigestUpdate(ctx, state_salt, strlen(state_salt));
    EVP_DigestFinal_ex(ctx, pool, &md_len); // 新しい状態で上書き
    
    EVP_MD_CTX_free(ctx);

    save_pool(pool);
    return 0;
}

int main() {
    uint8_t pool[POOL_SIZE];
    uint8_t output_key[OUTPUT_SIZE];

    if (load_pool(pool) != 0) {
        fprintf(stderr, "プールの読み込みに失敗しました。\n");
        return 1;
    }

    printf("1: サイコロの目を追加してプールを育てる\n");
    printf("2: 256ビット乱数（鍵）を切り出す\n");
    printf("選択してください (1/2): ");
    
    int choice;
    if (scanf("%d", &choice) != 1) return 1;
    getchar(); // 改行コードの読み飛ばし

    if (choice == 1) {
        char dice_input[1024];
        printf("サイコロの出目を入力（例: 351624）: ");
        if (fgets(dice_input, sizeof(dice_input), stdin)) {
            dice_input[strcspn(dice_input, "\n")] = 0;
            int count = feed_dice(pool, dice_input);
            printf("成功: %d個の出目をプールに撹拌しました。\n", count);
        }
    } else if (choice == 2) {
        extract_key(pool, output_key);
        printf("生成された256ビット乱数 (HEX):\n");
        print_hex(output_key, OUTPUT_SIZE);
    }

    // スタック上の秘密情報を完全に消去
    secure_memzero(pool, POOL_SIZE);
    secure_memzero(output_key, OUTPUT_SIZE);

    return 0;
}

