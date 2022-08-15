
#define _GNU_SOURCE
#include "config.h"

#include <math.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int hmac(const char *crypto, unsigned char *key, size_t keysiz,
         unsigned char *msg, size_t msglen, unsigned char *hash, size_t *s)
{
    // TODO: EVP_MAC が OpenSSL に直接導入されるのは ver. 3.0 以降
    // OpenSSL_add_all_algorithms();
    // OpenSSL_add_all_ciphers();
    // OpenSSL_add_all_digests();
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *md = EVP_MD_fetch(NULL, "SHA-1", NULL);
#else
    const EVP_MD *md = EVP_get_digestbyname("SHA-1");
#endif
    if (md == NULL)
        return 1;
    EVP_PKEY *pkey
        = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, (int)keysiz);
    EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey);
    EVP_DigestSignUpdate(mdctx, msg, msglen);
    EVP_DigestSignFinal(mdctx, hash, s);
    EVP_MD_CTX_free(mdctx);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(md);
#endif
    return 0;
}

static const int DIGITS_POWER[9]
    //  0  1   2    3     4      5       6        7         8
    = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

char *generateTOTP(unsigned char *key, size_t keysiz, time_t time,
                   int returnDigits, const char *crypto)
{
    uint64_t msgword = (uint64_t)time;
    unsigned char *msg = (unsigned char *)&msgword;

    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    size_t hashsiz = EVP_MAX_MD_SIZE;
    if (hmac(crypto, key, keysiz, msg, 8, hash, &hashsiz) != 0)
    {
        return NULL;
    }

    int offset = hash[hashsiz - 1] & 0x0f;

    int binary
        = ((hash[offset] & 0x7f) << 24) | ((hash[offset + 1] & 0xff) << 16)
          | ((hash[offset + 2] & 0xff) << 8) | (hash[offset + 3] & 0xff);

    int otp = binary % DIGITS_POWER[returnDigits];

    // テキスト整形
    char format[16] = "";
    snprintf(format, 16, "%%0%dd", returnDigits);
    char *result = malloc(returnDigits + 1);
    // 念のためゼロクリア
    memset(result, 0, returnDigits + 1);
    // OTP文字列生成
    snprintf(result, returnDigits + 1, format, otp);

    return result;
}

int hiho(int argc, char **argv, const char **envp)
{
    // execシリーズで自分自身を起動するとどうなるの
    pid_t pid = 0;
    printf("%p, %p\n", argv, argv[0]);
    int status = 0;
    size_t i = 0;

    if ((pid = fork()) == 0)
    {
        // 子プロセス
        char *localargv[] = { "/bin/echo", "hello world", NULL };
        char *localenvp[] = { NULL };
        execve(localargv[0], localargv, localenvp);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        // 親プロセス
        int ret = waitpid(pid, &status, 0);
        printf("%d %d %d\n", pid, ret, status);
    }
    else
    {
        // プロセス起動失敗
        perror("failed to fork");
        return EXIT_FAILURE;
    }
    return status;
}
