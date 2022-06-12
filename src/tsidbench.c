
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

#define PUBLIC_KEY "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"

struct a
{
    int tid;
    pthread_barrier_t *barrier;
};

void *function(void *arg)
{
    struct a *a = (struct a *)arg;
    size_t i = 88172645463325252UL;
    const size_t len = strlen(PUBLIC_KEY);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    const EVP_MD *sha1 = EVP_sha1();
    char count[24];
    unsigned char hash[20];
    int zerobyte = 0;
    int max = 0;
    size_t max_i = 0;
    uint64_t tmp = 0;

    pthread_barrier_wait(a->barrier);
    int length = 0;
    //    for (; a->tid; i++)
    for (; max < 6; i++)
    {
        EVP_DigestInit(&ctx, sha1);
        EVP_DigestUpdate(&ctx, PUBLIC_KEY, len);
        length = snprintf(count, 24, "%zu", i);
        EVP_DigestUpdate(&ctx, count, length);
        EVP_DigestFinal(&ctx, hash, NULL);
        tmp = htobe64(*(unsigned long *)hash);
        zerobyte =((tmp == 0) ? 64UL : __builtin_clzl(tmp));
        if (zerobyte > max)
        {
            // 更新したら置き換える
            max = zerobyte;
            max_i = i;
        }
    }
    EVP_MD_CTX_free(&ctx);
    printf("%lu, %lu\n", max, max_i);
    printf("%lu\n", i);
    return NULL;
}

#define THREAD_NUM 8

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
    pthread_t threads;
    struct a a;
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, 2);

    a.tid = 1;
    a.barrier = &barrier;
    pthread_create(&threads, NULL, function, &a);
    pthread_barrier_wait(&barrier);
    sleep(10);
    a.tid = 0;
    pthread_join(threads, NULL);
    pthread_barrier_destroy(&barrier);
    return EXIT_SUCCESS;
}
