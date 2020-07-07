
#include "study-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <orz.h>
#include <openssl/evp.h>
#include <java_random.h>
#include <err.h>

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

char *base58encode(unsigned char *input, size_t length)
{
  if (input == NULL || length == 0)
  {
    return NULL;
  }
  unsigned char *work = alloca(length);
  memset(work, 0, length);
  memcpy(work, input, length);
  size_t zeroCount = 0;
  while (zeroCount < length && work[zeroCount] == 0)
  {
    zeroCount++;
  }
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
  char *output = malloc(templen - j);
  memset(output, 0, templen - j);
  memcpy(output, &temp[j], templen - j);
  char *tmp = realloc(output, strlen(output));
  if(!tmp)
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
    *((uint16_t *)&p->data[1]) = (uint16_t)u;
    p->length = 2;
    return p;
  }
  if (65536 <= u && u < 4294967296L)
  {
    p->data = malloc(sizeof(char) + sizeof(uint32_t));
    *p->data = 254;
    *((uint32_t *)&p->data[1]) = (uint32_t)u;
    p->length = 2;
    return p;
  }
  if (4294967296L <= u && u <= 18446744073709551615UL)
  {
    p->data = malloc(sizeof(char) + sizeof(uint64_t));
    *p->data = 254;
    *((uint64_t *)&p->data[1]) = (uint64_t)u;
    p->length = 2;
    return p;
  }
  free(p);
  return NULL;
}

void chararrayfree(struct chararray *p)
{
  if (p == NULL)
  {
    return;
  }
  if (p->data != NULL)
  {
    free(p->data);
    p->data = NULL;
  }
  free(p);
}

char *encodeAddress(int version, int stream, unsigned char *ripe, size_t ripelen)
{
  unsigned char *workripe = NULL;
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
      workripelen = 18;
    }
    if (memcmp(ripe, "\0", 1) == 0)
    {
      workripe = &ripe[1];
      workripelen = 19;
    }
  }
  else
  {
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
  char *tmp = realloc(ret, strlen(ret) + 1);
  if(!ret)
  {
    free(ret);
    err(EXIT_FAILURE, "can not realloc");
  }
  ret = tmp;

  return tmp;
}

/*
 * strを16進数文字列としてパースします。
 */
static size_t parseHex(unsigned char **out, const char *str)
{
    size_t length = strlen(str) / 2;
    size_t i = 0;
    unsigned char *data = calloc(length, sizeof(char));
    static char table[256] = {0};
    if (table[0x30] == 0)
    {
        for (i = 0; i < 10; i++)
        {
            table[(0x30 + i)] = (char)i;
        }
        for (i = 0; i <= 6; i++)
        {
            table[(0x40 + i)] = (char)(9 + i);
        }
        for (i = 0; i <= 6; i++)
        {
            table[(0x60 + i)] = (char)(9 + i);
        }
    }
    if (!data)
    {
        perror(NULL);
        exit(1);
    }
    for (i = 0; i < length; i++)
    {
        data[i] = (table[str[2 * i]] << 4) | (table[str[2 * i + 1]]);
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
  char *hex = "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5";
  unsigned char *in = NULL;
  len = (unsigned int)parseHex(&in, hex);
  printf("%u\n", len);
  //memset(in, 0, 10);
  char *out = base58encode(in, len);
  printf("%s\n", out);
  free(in);
  free(out);
  return EXIT_SUCCESS;
}
