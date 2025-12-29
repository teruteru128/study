
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define PUBLIC_KEY "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"

static pthread_barrier_t barrier;
static pthread_spinlock_t spin;

volatile int con = 1;
static int max = 0;
static uint64_t max_i = 0;
static EVP_MD *sha1 = NULL;
static char *public_key = NULL;

static inline int fast_utoa(uint64_t val, char *buf)
{
    char temp[20];
    int i = 0;
    if (val == 0)
    {
        buf[0] = '0';
        return 1;
    }
    while (val > 0)
    {
        temp[i++] = (val % 10) + '0';
        val /= 10;
    }
    int len = i;
    while (i > 0)
    {
        buf[len - i] = temp[i - 1];
        i--;
    }
    return len;
}

void *function(void *arg)
{
    uint64_t i = ((uint64_t *)arg)[0];
    const uint64_t step = ((uint64_t *)arg)[1];

    const size_t len = strlen(public_key);
    EVP_MD_CTX *common_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex2(common_ctx, sha1, NULL);
    EVP_DigestUpdate(common_ctx, public_key, len);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    char count[24];
    unsigned char hash[20];
    int zerobyte = 0;
    uint64_t tmp = 0;

    int length = 0;
    pthread_barrier_wait(&barrier);
    while (con)
    {
        EVP_MD_CTX_copy_ex(ctx, common_ctx);
        length = fast_utoa(i, count);
        EVP_DigestUpdate(ctx, count, length);
        EVP_DigestFinal_ex(ctx, hash, NULL);
        tmp = htobe64(*(uint64_t *)hash);
        zerobyte = ((tmp == 0) ? 64UL : __builtin_clzl(tmp));
        // 1次チェック（ロックなし：ほとんどがここで弾かれる）
        if (zerobyte > *(volatile int *)&max)
        {
            pthread_spin_lock(&spin);
            // 2次チェック（ロック取得後に確定）
            if (zerobyte > max)
            {
                // 更新したら置き換える
                max = zerobyte;
                max_i = i;
            }
            pthread_spin_unlock(&spin);
        }
        i += step;
    }
    EVP_MD_CTX_free(ctx);
    EVP_MD_CTX_free(common_ctx);
    printf("%zu\n", i);
    return NULL;
}

#define THREAD_NUM 16

/**
 *
 * 対称鍵暗号
 *   EVP_CIPHER
 *   https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
 * 認証付き暗号
 *   https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption
 * エンベロープ暗号化(ハイブリッド暗号？)
 *   https://wiki.openssl.org/index.php/EVP_Asymmetric_Encryption_and_Decryption_of_an_Envelope
 * 署名と検証
 *   EVP_DigestSign
 *   https://wiki.openssl.org/index.php/EVP_Signing_and_Verifying
 * メッセージダイジェスト
 *   https://wiki.openssl.org/index.php/EVP_Message_Digests
 * 鍵合意(鍵交換)
 *   EVP_PKEY_CTX
 *   EVP_PKEY_derive
 *   https://wiki.openssl.org/index.php/EVP_Key_Derivation
 * メッセージ認証符号 (OpenSSL 3～)
 *   EVP_MAC_new_ctx
 * 鍵導出関数
 *   EVP_KDF
 *   https://wiki.openssl.org/index.php/EVP_Key_Derivation
 * strsep
 *   トークン分割(空フィールド対応版)
 * versionsort
 * strverscmp
 * alphasort
 * tor geoip file 読み込み関数
 * geoip_load_file
 * https://youtu.be/MCzo6ZMfKa4
 * ターミュレーター
 *
 * regex.h
 * - 最左最短一致
 * - 否定先読み
 * - 強欲な数量子
 *
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 *
 * 標準入力と標準出力を別スレッドで行うアプリ
 * リクエストを投げると適当なデータを投げ返す簡単なサーバープログラム
 * リクエスト長さは8バイトに対応
 *
 * --help
 * --version
 * --server-mode
 *   フォアグラウンドで起動
 * --daemon-mode
 *   デーモン化処理付きでバックグラウンドで起動
 *
 * 複数スレッドをpthread_cond_tで止めてメインスレッドでtimerfdを使って指定時刻まで待ち、pthread_cond_broadcastで一斉に起動する
 *
 * ファイルからGMPのmpzに整数を読み込んだりOpenSSLのBIGNUMに整数を読み込んだり乱数を読み込んだりを共通化したい
 *
 * @brief sanbox func.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    uint64_t init = 0;
    uint32_t sl = 16;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--init") == 0 && i + 1 < argc)
        {
            init = strtoull(argv[i + 1], NULL, 10);
            i++;
        }
        else if (strcmp(argv[i], "--max") == 0 && i + 1 < argc)
        {
            max = (int)strtoll(argv[i + 1], NULL, 10);
            i++;
        }
        else if (strcmp(argv[i], "--public-key-file") == 0 && i + 1 < argc)
        {
            FILE *in = fopen(argv[i + 1], "r");
            char buffer[BUFSIZ];
            if (in != NULL)
            {
                char *tmp = fgets(buffer, BUFSIZ, in);
                if (tmp == NULL)
                {
                    perror("fgets");
                    return EXIT_FAILURE;
                }
                tmp[strcspn(tmp, "\r\n")] = '\0';
                char *tmp_dup = strdup(buffer);
                if (tmp_dup == NULL)
                {
                    perror("strdup");
                    return EXIT_FAILURE;
                }
                public_key = tmp_dup;
            }
        }
        else if (strcmp(argv[i], "--public-key-file") == 0 && i + 1 < argc)
        {
            sl = (uint32_t)strtoll(argv[i + 1], NULL, 10);
            i++;
        }
    }
    if (public_key == NULL)
    {
        fprintf(stderr, "公開鍵ファイルを指定してください\n");
        return EXIT_FAILURE;
    }
    printf("public_key: %s\n", public_key);
    sha1 = EVP_MD_fetch(NULL, "SHA-1", NULL);
    pthread_t threads[THREAD_NUM];
    pthread_barrier_init(&barrier, NULL, THREAD_NUM + 1);
    pthread_spin_init(&spin, 0);
    uint64_t arg[THREAD_NUM * 2];

    for (int i = 0; i < THREAD_NUM; i++)
    {
        arg[i * 2] = init + i;
        arg[i * 2 + 1] = THREAD_NUM;
        pthread_create(threads + i, NULL, function, &arg[i * 2]);
    }
    pthread_barrier_wait(&barrier);
    sleep(sl);
    con = 0;
    for (int i = 0; i < THREAD_NUM; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("%d, %zu\n", max, max_i);
    pthread_spin_destroy(&spin);
    pthread_barrier_destroy(&barrier);
    EVP_MD_free(sha1);
    free(public_key);
    return EXIT_SUCCESS;
}
