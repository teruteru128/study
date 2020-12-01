
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double chainCookies(long chain, double cookiesPs, double cookies)
{
  printf("chain!");
  double digit = 7;
  double mult = 1;
  chain++;
  if (chain <= 1 && chain >= 1)
    chain += (long)fmax(0, ceil(log10(cookies)) - 10);
  double maxPayout = fmin(cookiesPs * 60 * 60 * 6, cookies * 0.5) * mult;
  double moni = fmax(digit, fmin(floor(1. / 9 * pow(10, (double)chain) * digit * mult), maxPayout));
  double nextMoni = fmax(digit, fmin(floor(1. / 9 * pow(10, (double)(chain + 1)) * digit * mult), maxPayout));
  printf("%ld, %e, %e, %e\n", chain, maxPayout, moni, nextMoni);
  if (nextMoni >= maxPayout)
  {
    printf("Cookie chain over.\n");
  }
  return moni;
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
  long chain = 30;
  double cookiesPs = 5.587e+26;
  double cookies = 5.877e+32;
  printf("%f\n", chainCookies(chain, cookiesPs * 7, cookies));
  return EXIT_SUCCESS;
}
