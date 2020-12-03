
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>
#define SRC "551 5 ABCDEFG:2005/03/27 12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6,仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

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

#define strregerr(r) fputs(#r "\n", stderr)

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
  /*
  long chain = 30;
  double cookiesPs = 5.587e+26;
  double cookies = 5.877e+32;
  printf("%f\n", chainCookies(chain, cookiesPs * 7, cookies));
  printf("%f = 10^%f\n", 15., log10(15));
  */
  /* 
   * y=x/(1+x)
   * y(1+x)=x
   * y + xy = x
   * y=x-xy
   * y=x(1-y)
   * x=y/(1-y)
   * x=y/(4-y)
   * x(4-y) = y
   * 4x -xy = y
   * y + xy = 4x
   * y(1+x)=4x
   * y=4x/(1+x)
   * パラメータ一覧
   * - 棒とか玉のサイズ
   * - 快感の強さ
   * - 射精量スケール
   * - 安全リミット
   */
  /*
  double p = 0;
  const double maxp = log10(2000);
  for (double i = 0; i <= 1000; i++)
  {
    p = fmin(6 * (i / 100.0) / (2 + (i / 100.0)), maxp);
    printf("p : %f, %fL\n", p, pow(10, p) / 1000);
  }
  */
  int code;
  int hop;
  char format[BUFSIZ];
  snprintf(format, BUFSIZ, "%%d %%d %%%ds", BUFSIZ - 1);
  char buf[BUFSIZ];
  // sscanfは空白文字を読めないので不可
  sscanf(SRC, format, &code, &hop, buf);
  printf("%d, %d, %s\n", code, hop, buf);
  regex_t reg;
  /**
   * @brief Construct a new regex object
   * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
   * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
   * "^[[:digit:]]{3}( [[:digit:]]+( .*)?)?$"
   * REG_NEWLINEはCRを除外しない
   */
  int r = regcomp(&reg, "^([[:digit:]]{3}) ([[:digit:]]+)( (.+))?$", REG_EXTENDED | REG_NEWLINE);
  if (r != 0)
  {
    switch (r)
    {
    case REG_BADRPT:
      fprintf(stderr, "badrpt %d\n", r);
      break;
    default:
      fprintf(stderr, "other %d\n", r);
      break;
    }
    return EXIT_FAILURE;
  }
  regmatch_t match[5];
  if (regexec(&reg, SRC, 5, match, 0) == 0)
  {
    printf("matched : %d, %d\n", match[4].rm_so, match[4].rm_eo);
    strncpy(buf, SRC + match[4].rm_so, match[4].rm_eo - match[4].rm_so);
    char *p = strpbrk(buf, "\r\n");
    if (p != NULL)
    {
      *p = '\0';
    }
    printf("%s\n", buf);
  }
  regfree(&reg);
  return EXIT_SUCCESS;
}
