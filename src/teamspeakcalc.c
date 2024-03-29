
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(str) gettext(str)

#include "java_random.h"
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <omp.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/sha.h>
#include <printint.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define IN2_SIZE 21

#define ANDROID_IDENTITY                                                      \
    "MEwDAgcAAgEgAiBK4dcDZUSLCxmvRfMWMAQf1JzSrLzZakLqDsULzT28OwIhAILbBS66JoN" \
    "1Xo2YsC1xDPDhukJjVO2guoeL+AM27Vfn"
#define DEFAULT_IDENTITY                                                      \
    "MEwDAgcAAgEgAiEA/4wiwfLIWgVHTUmqF9f6n1FzsvaO/"                           \
    "80YoDP91I+NB78CIGo3pZSOCKIz6mCKSS1POh5TN1tuSdgtLxcnsjG6iVTr"
#define MAIN_IDENTITY                                                         \
    "MEwDAgcAAgEgAiEA7Vo1+"                                                   \
    "Orf2xuuu6hTPAPldSfrUZZ7WYAzpRcO5DoYFLoCIF1JKVBctOGvMOy495O/"             \
    "BWFuFEYH4i1f6vU0b9+a64RD"
#define NEW_IDENTITY                                                          \
    "MEwDAgcAAgEgAiEA+i4ptdb7Q5ldNJjyJTd/+hC+ac2YoPoIXYLgPRJE6egCIBcdWTjBr/"  \
    "iW3QjAAl389HYDZF/0GwuxH+MpXdDBrpl0"
#define THIRD_IDENTITY                                                        \
    "MEsDAgcAAgEgAiAoQPNcS7L4k+q2qf3U7uyujtwRQNS3pLKN/"                       \
    "zrRGERGagIgFjdV1JlqHF8BiIQne0/E3jVM7hWda/USrFI58per45s="

/**
 * @brief ルーチン
 * 
 * @param publickey 
 * @param start_v 
 * @param finish_v 
 */
void routine(const char *publickey, const uint64_t start_v,
             const uint64_t finish_v)
{
    printf("Public key: %s\n", publickey);
    printf("Search range: %" PRIu64 " -> %" PRIu64 "\n", start_v, finish_v);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *sha1 = EVP_MD_fetch(NULL, "SHA-1", NULL);
#else
    const EVP_MD *sha1 = EVP_sha1();
#endif
    EVP_MD_CTX *srcctx = EVP_MD_CTX_new();
    EVP_MD_CTX *workctx = NULL;
    unsigned char md[EVP_MAX_MD_SIZE];
    int i = 0;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_DigestInit_ex2(srcctx, sha1, NULL);
#else
    EVP_DigestInit_ex(srcctx, sha1, NULL);
#endif
    EVP_DigestUpdate(srcctx, publickey, strlen(publickey));
    // 配列長さ
    char counter_buffer[IN2_SIZE];
    uint64_t verifier = 0;
    int clz = -1;
#pragma omp parallel private(workctx, md, i, counter_buffer, clz)
    {
        workctx = EVP_MD_CTX_new();
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_DigestInit_ex2(workctx, sha1, NULL);
#else
        EVP_DigestInit_ex(workctx, sha1, NULL);
#endif
        // one shotフラグを使ってまとめてupdateするよりcopyしたほうが早い
#pragma omp for
        for (verifier = start_v; verifier < finish_v; verifier++)
        {
            // 公開鍵の末尾にverifierを書き込み
            // SHA1でハッシュを作成
            EVP_MD_CTX_copy_ex(workctx, srcctx);
            EVP_DigestUpdate(
                workctx, counter_buffer,
                snprintf(counter_buffer, IN2_SIZE, "%" PRIu64, verifier));
            EVP_DigestFinal_ex(workctx, md, NULL);

            // if (memcmp(md, "\0\0\0\0\0", 3) == 0)
            // 念のため先頭64ビットが0の場合に備える
            clz = (*(uint64_t *)md == 0)
                      ? 64
                      : __builtin_ctzl(le64toh(*(uint64_t *)md));
            if (clz >= 40)
            {
#pragma omp critical
                {
                    printf(_("verifier : %d(%" PRIu64 ") "), clz, verifier);
                    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
                    {
                        printf("%02x", md[i]);
                    }
                    printf("\n");
                }
            }
        }
        EVP_MD_CTX_free(workctx);
        workctx = NULL;
    }
    EVP_MD_CTX_free(srcctx);
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(sha1);
#else
    // Do nothing because EVP_MD is const
#endif
}

/**
 * ANDROID_IDENTITY: 0x10000000000UL まで完
 * DEFAULT_IDENTITY: 0x50000000000UL まで完
 * MAIN_IDENTITY: 0x100000000000UL まで完
 * NEW_IDENTITY: 0x2A000000000UL まで完
 * THIRD_IDENTITY: 0x34000000000UL まで完
 * 0x02000000000を 8スレ->ほぼ1時間
 * 0x20000000000を16スレ->37610s(11.4h)
 * 0x2A200000000を16スレ->15h?
 */
int main(const int argc, const char *argv[])
{
    const char *publicKey = (argc >= 2) ? argv[1] : MAIN_IDENTITY;
    struct tm tm = { 0 };
    char timebuf[512] = "";

    time_t start = 0;
    time_t finish = 0;
    for (size_t start_v = 0xD8000000000UL, finish_v = start_v + 0x2000000000UL;
         start_v < 0x100000000000UL;
         start_v += 0x2000000000UL, finish_v += 0x2000000000UL)
    {
        start = time(NULL);
        localtime_r(&start, &tm);
        strftime(timebuf, 512, "%Y/%m/%d %T", &tm);
        printf("開始: %s\n", timebuf);
        routine(publicKey, start_v, finish_v);
        finish = time(NULL);
        localtime_r(&finish, &tm);
        strftime(timebuf, 512, "%Y/%m/%d %T", &tm);
        printf("終わり！: %s(%" PRId64 ")\n", timebuf,
               (int64_t)difftime(finish, start));
    }
    return EXIT_SUCCESS;
}
