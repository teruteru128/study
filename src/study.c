
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "gettext.h"
#define _(str) gettext(str)
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <byteswap.h>
#include <inttypes.h>
#include <stddef.h>
#include <openssl/evp.h>

#define LIMIT 16

typedef struct rndctx
{
  uint64_t count;
  unsigned char ctx[20];
} rndctx_t;

int64_t next(rndctx_t *ctx)
{
  const EVP_MD *sha1 = EVP_sha1();
  // 毎回ctxをnewしてfreeするのもったいないけどなにかいい方法ない？
  EVP_MD_CTX *tmpmd = EVP_MD_CTX_new();
  EVP_DigestInit(tmpmd, sha1);
  *((uint64_t *)(ctx->ctx + 12)) = le64toh(ctx->count);
  ctx->count++;
  EVP_DigestUpdate(tmpmd, ctx->ctx, 20);
  EVP_DigestFinal(tmpmd, ctx->ctx, NULL);
  EVP_MD_CTX_free(tmpmd);
  return  *((uint64_t *)(ctx->ctx + 12));
}

/**
 * --help
 * --verbose
 * --version
 * 
 * orz
 * 
 * OpenSSL EVP
 * 対称鍵暗号
 * 認証付き暗号
 * エンベロープ暗号化
 * 署名と検証
 *   EVP_DigestSign
 * メッセージダイジェスト
 * 鍵合意(鍵交換)
 * メッセージ認証符号 (OpenSSL 3～)
 *   EVP_MAC_new_ctx
 * 鍵導出関数
 * strpbrk
 *   文字検索関数
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
 * TODO: P2P地震情報 ピア接続受け入れ＆ピアへ接続
 */
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  /*
    SHA-1ベース乱数
    内部状態 12バイト乱数+8バイトカウンタ(0スタート)
    初期化 12バイト乱数
    sha1(12バイト乱数+8バイトカウンタ)
    →12バイト再利用+8バイト出力
    カウンタをctxに上書き
    カウンタをインクリメント
    ハッシュしてctxに書き込む
    出力値を返す
  */
  rndctx_t ctx;
  ctx.count = 0;
  unsigned char buf[20];
  FILE *r = fopen("/dev/urandom", "rb");
  if (r == NULL)
  {
    return EXIT_FAILURE;
  }
  size_t req = 12;
  size_t len = fread(buf, sizeof(char), req, r);
  if (len != req)
  {
    perror("fread error1");
  }
  len = fread(ctx.ctx, sizeof(char), req, r);
  if (len != req)
  {
    perror("fread error2");
  }
  fclose(r);
  *((uint64_t *)(buf + 12)) = le64toh(1);
  for (size_t i = 0; i < 20; i++)
  {
    printf("%02x", buf[i]);
  }
  fputs("\n", stdout);
  for(int i = 0; i < 20; i++)
  {
    printf("%ld\n", next(&ctx));
  }
  int h = 0;
  int v = 0;
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--help") == 0)
    {
      h = 1;
    }
    else if (strcmp(argv[i], "--version") == 0)
    {
      v = 1;
    }
  }
  printf("%d %d\n", h, v);
  // メインコマンドのhelpとサブコマンドのhelpを別に実装するには？
  fputs("ひぐらしの\e[31mな\e[mく頃に\n", stdout);
  return EXIT_SUCCESS;
}
