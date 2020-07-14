
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <orz.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <java_random.h>
#include <err.h>
#include <byteswap.h>

#define BIG_PRECURE_IS_WATCHING_YOU "BIG PRECURE IS WATCHING YOU"

#define ALPHABET "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
#define BASE_58 58

static char divmod58(unsigned char *number, size_t length, size_t startAt)
{
  int remainder = 0;
  for (size_t i = startAt; i < length; i++)
  {
    int digit256 = number[i] & 0xFF;
    int temp = (remainder << 8) + digit256;

    number[i] = (char)(temp / BASE_58);

    remainder = temp % BASE_58;
  }

  return (char)remainder;
}

/*
 * どうやったらこんな使い勝手のいいライブラリが作れるんですか？
 * https://github.com/Bitmessage/PyBitmessage/blob/8659c5313d54ea8c1f899f8a4e0bc8c02ae5dab3/src/pyelliptic/arithmetic.py#L41
 * そもそもPythonのencode関数の引数valが多倍長整数臭いんだよねえ……
 * →Python 2.x系はint型が少なくとも32ビット精度の整数、longが多倍長整数
 * Python　3.x系だとintが多倍長整数 https://docs.python.org/ja/2.7/library/stdtypes.html#numeric-types-int-float-long-complex
 * C言語だとmpz_tとかBIGNUMとかの使用が前提やぞ
 * def encode(val, base, minlen=0):
 * def decode(string, base):
 * def changebase(string, frm, to, minlen=0):
 * 
 * 生バイナリを渡して文字列に変換する
 * char *encode(char *, size_t len, int base, size_t minlen);
 * 
 * 文字列を渡してバイナリに変換する
 * 本当にNULL終端された文字列だと仮定していいのか？
 * NULL終端されていないと仮定して文字列長も受け取ったほうが便利じゃないか？
 * stringの実際の長さと引数lenで渡した文字列長が一致しない場合は……呼び出し側の責任なので問題ないのか
 * そもそも返すバイナリの長さがわからないのが問題だな？
 * データ長をデータ本体ともども引数経由で返して返り値はエラー判定に使うか？
 * char *decode(char *string, int base);
 * int decode(char *string, size_t stringlen, int base, char **val, size_t *vallen);
 * char *changebase(char *string, size_t len, int from, int to, size_t minlen);
 * int changebase(char *string, size_t len, int from, int to, size_t minlen, char **val, size_t *vallen);
 * 
 * JavaとかPythonとかだと配列にlengthが付いてるから楽でいいなあ……
 * 
 * そもそも文字列と生バイナリの区別がデータ型でできないのが割と辛い
 * ……Javaでも変わらんか
 * 
 * バイナリがunsigned charで文字列が(signed) charをよく使ってる気がする
 * 
 * iconv(3)に習うならoutputのbufも呼び出し側が指定すべきなんだよな
 * iconvの実装が古い可能性
 * 2, 10, 16, 58, 256
 * 
 * 生バイナリ(256進数)→58進数:1.365倍
 * 生バイナリ(256進数)→16進数:2倍の領域
 * 生バイナリ(256進数)→10進数:2.408倍の領域
 * 生バイナリ(256進数)→10進数:8倍の領域
 * 58進数→16進数:1.464倍の領域
 * 58進数→10進数:1.763倍の領域
 * 58進数→2進数:5.858倍の領域
 * 16進数→10進数:1.204倍の領域
 * 16進数→2進数:4倍の領域
 * 10進数→2進数:3.322倍の領域
 * 計算めんどくさいし誤差でオーバーランしたら嫌だし
 * 小数点以下切り上げでとりあえず確保してreallocいいんじゃないかな( ˘ω˘)
 * ->ローカル変数で領域を確保してstrdupするなりなんなりでもいい気がする……文字列ならな
 * 
 * encodeBase58
 * https://github.com/Bitmessage/PyBitmessage/blob/d09782e53d3f42132532b6e39011cd27e7f41d25/src/addresses.py#L14
 * 
 * decodeBase58
 * https://github.com/Bitmessage/PyBitmessage/blob/d09782e53d3f42132532b6e39011cd27e7f41d25/src/addresses.py#L33
 * 
 */
char *base58encode(unsigned char *input, size_t length)
{
  if (input == NULL || length == 0)
  {
    return NULL;
  }
  unsigned char *work = alloca(length);
  memcpy(work, input, length);
  size_t zeroCount = 0;
  while (zeroCount < length && work[zeroCount] == 0)
    zeroCount++;
  size_t templen = length * 2;
  char *temp = alloca(templen);
  size_t j = templen;
  memset(temp, 0, j);

  size_t startAt = zeroCount;
  while (startAt < length)
  {
    int mod = divmod58(work, length, startAt);
    if (work[startAt] == 0)
    {
      ++startAt;
    }

    temp[--j] = ALPHABET[mod];
  }
  while (j < templen && temp[j] == ALPHABET[0])
  {
    ++j;
  }
  while (zeroCount--)
  {
    temp[--j] = ALPHABET[0];
  }
  size_t outputlen = templen - j;
  char *output = malloc(outputlen);
  memset(output, 0, outputlen);
  memcpy(output, &temp[j], outputlen);
  char *tmp = realloc(output, strlen(output));
  if (!tmp)
  {
    free(output);
    err(EXIT_FAILURE, "realloc in base58encode");
  }
  output = tmp;
  return output;
}

struct chararray
{
  char *data;
  size_t length;
};

/*
 * 
 * https://github.com/Bitmessage/PyBitmessage/blob/d09782e53d3f42132532b6e39011cd27e7f41d25/src/addresses.py#L63
 * https://docs.python.org/ja/3/library/struct.html
 */
struct chararray *encodeVarint(uint64_t u)
{
  struct chararray *p = malloc(sizeof(struct chararray));
  if (p == NULL)
  {
    return NULL;
  }
  if (u < 253)
  {
    p->data = malloc(sizeof(char));
    *p->data = (uint8_t)u;
    p->length = 1;
    return p;
  }
  if (253 <= u && u < 65536)
  {
    p->data = malloc(sizeof(char) + sizeof(uint16_t));
    *p->data = 253;
    *((uint16_t *)&p->data[1]) = bswap_16((uint16_t)u);
    p->length = 3;
    return p;
  }
  if (65536 <= u && u < 4294967296L)
  {
    p->data = malloc(sizeof(char) + sizeof(uint32_t));
    *p->data = 254;
    *((uint32_t *)&p->data[1]) = bswap_32((uint32_t)u);
    p->length = 5;
    return p;
  }
  if (4294967296L <= u && u <= 18446744073709551615UL)
  {
    p->data = malloc(sizeof(char) + sizeof(uint64_t));
    *p->data = 254;
    *((uint64_t *)&p->data[1]) = bswap_64((uint64_t)u);
    p->length = 9;
    return p;
  }
  // おそらくここには来ない
  free(p);
  return NULL;
}

void chararrayfree(struct chararray *p)
{
  if (p->data)
    free(p->data);
  free(p);
}

/*
 * ripeをBitMessageアドレスにエンコードします。
 * 
 * https://github.com/Bitmessage/PyBitmessage/blob/d09782e53d3f42132532b6e39011cd27e7f41d25/src/addresses.py#L143
 * https://github.com/teruteru128/java-study/blob/03906187223ad8e5e8f8629e23ecbe2fbca5b7b4/src/main/java/com/twitter/teruteru128/study/bitmessage/genaddress/BMAddress.java#L18
 */
char *encodeAddress(int version, int stream, unsigned char *ripe, size_t ripelen)
{
  unsigned char *workripe = ripe;
  size_t workripelen = ripelen;
  if (version >= 2 && version < 4)
  {
    if (ripelen != 20)
    {
      return NULL;
    }
    if (memcmp(ripe, "\0\0", 2) == 0)
    {
      workripe = &ripe[2];
      workripelen -= 2;
    }
    else if (memcmp(ripe, "\0", 1) == 0)
    {
      workripe = &ripe[1];
      workripelen -= 1;
    }
  }
  else
  {
    if (ripelen != 20)
    {
      return NULL;
    }
    size_t i = 0;
    for (; ripe[i] == 0 && i < ripelen; i++)
      ;

    workripe = &ripe[i];
    workripelen -= i;
  }
  struct chararray *variantVersion = encodeVarint(version);
  struct chararray *variantStream = encodeVarint(stream);
  size_t storedBinaryDataLen = variantVersion->length + variantStream->length + workripelen + 4;
  unsigned char *storedBinaryData = malloc(variantVersion->length + variantStream->length + workripelen + 4);
  memcpy(storedBinaryData, variantVersion->data, variantVersion->length);
  memcpy(storedBinaryData + variantVersion->length, variantStream->data, variantStream->length);
  memcpy(storedBinaryData + variantVersion->length + variantStream->length, workripe, workripelen);
  chararrayfree(variantVersion);
  chararrayfree(variantStream);

  {
    const EVP_MD *sha512 = EVP_sha512();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, storedBinaryData, storedBinaryDataLen - 4);
    unsigned int s = 0;
    unsigned char cache64[64];
    EVP_DigestFinal(ctx, cache64, &s);
    EVP_DigestInit(ctx, sha512);
    EVP_DigestUpdate(ctx, cache64, 64);
    EVP_DigestFinal(ctx, cache64, &s);
    EVP_MD_CTX_free(ctx);
    memcpy(storedBinaryData + variantVersion->length + variantStream->length + workripelen, cache64, 4);
  }

  char *a = base58encode(storedBinaryData, storedBinaryDataLen);
  free(storedBinaryData);
  char *ret = malloc(45);
  snprintf(ret, 45, "BM-%s", a);
  free(a);
  {
    char *tmp = realloc(ret, strlen(ret) + 1);
    if (!ret)
    {
      free(ret);
      err(EXIT_FAILURE, "can not realloc");
    }
    ret = tmp;
  }
  return ret;
}

#define TABLE "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\0\0\0\0\0\0" \
              "\0\x0a\x0b\x0c\x0d\x0e\x0f\0\0\0\0\0\0\0\0\0"         \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\x0a\x0b\x0c\x0d\x0e\x0f\0\0\0\0\0\0\0\0\0"         \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"                     \
              "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

/*
 * strを16進数文字列としてパースします。
 * jsonパーサ
 * https://mattn.kaoriya.net/software/lang/c/20130710214647.htm
 */
static size_t parseHex(unsigned char **out, const char *str)
{
  size_t length = strlen(str) / 2;
  size_t i = 0;
  unsigned char *data = calloc(length, sizeof(char));
  if (!data)
  {
    perror("calloc in parseHex");
    exit(1);
  }
  for (i = 0; i < length; i++)
  {
    data[i] = (TABLE[str[2 * i] & 0xff] << 4) | (TABLE[str[2 * i + 1] & 0xff]);
  }
  *out = data;
  return length;
}

/**
 * --version
 * --help
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
 */
int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  printf(_("Help me!\n"));
  orz(1);
  const EVP_MD *sha512 = EVP_sha512();
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, BIG_PRECURE_IS_WATCHING_YOU, strlen(BIG_PRECURE_IS_WATCHING_YOU));
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int len = 0;
  EVP_DigestFinal(ctx, md, &len);
  for (unsigned int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_DigestInit(ctx, sha512);
  EVP_DigestUpdate(ctx, md, len);
  EVP_DigestFinal(ctx, md, &len);
  for (unsigned int i = 0; i < len; i++)
  {
    printf("%02x", md[i]);
  }
  printf("\n");
  EVP_MD_CTX_free(ctx);
  int64_t ppp = 0x0;
  ppp = nInverse(ppp);
  printf("%ld\n", initialScramble(ppp));
  ppp = 246345201500483L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = 74803317123181L;
  ppp = initialScramble(ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = n(ppp);
  printf("246 : %012lx\n", ppp);
  ppp = 74803317123181L;
  ppp = initialScramble(ppp);
  printf("246 : %016lx\n", nextLong(&ppp));
  printf("246 : %016lx\n", nextLong(&ppp));
  ppp = 74803317123181L;
  ppp = initialScramble(ppp);
  printf("246 : %016lx\n", ((int64_t)next(&ppp, 32)) << 32);
  printf("246 : %016lx\n", (int64_t)next(&ppp, 32));
  printf("246 : %016lx\n", ((int64_t)next(&ppp, 32)) << 32);
  printf("246 : %016x\n", next(&ppp, 32));
  // 00000D9663F57318B4E52288BFDC8B3C23E84DE1
  char *hex = "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5";
  unsigned char *in = NULL;
  len = (unsigned int)parseHex(&in, hex);
  printf("%u\n", len);
  //memset(in, 0, 10);
  char *out = base58encode(in, len);
  printf("%s\n", out);
  free(in);
  free(out);
  char *hex2 = "00000D9663F57318B4E52288BFDC8B3C23E84DE1";
  in = NULL;
  len = (unsigned int)parseHex(&in, hex2);
  printf("%u\n", len);
  //memset(in, 0, 10);
  out = encodeAddress(4, 1, in, len);
  printf("%s\n", out);
  free(in);
  free(out);
  return EXIT_SUCCESS;
}
