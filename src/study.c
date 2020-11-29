
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX 134217728
#define THREAD_NUM 32

size_t sum;
pthread_mutex_t mutex;

void *func(void *arg)
{
  for (size_t i = 0; i < MAX; i++)
  {
    pthread_mutex_lock(&mutex);
    sum++;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

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
 * 
 * @brief sanbox func.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
  pthread_t threads[THREAD_NUM];
  pthread_mutex_init(&mutex, NULL);
  sum = 0;
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_create(&threads[i], NULL, func, NULL);
  }
  for (size_t i = 0; i < THREAD_NUM; i++)
  {
    pthread_join(threads[i], NULL);
  }
  pthread_mutex_destroy(&mutex);
  printf("%zu\n", sum);
  return EXIT_SUCCESS;
}
