
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

#define DEFAULT_IDENTITY                                                      \
    "MEwDAgcAAgEgAiEA/4wiwfLIWgVHTUmqF9f6n1FzsvaO/"                           \
    "80YoDP91I+NB78CIGo3pZSOCKIz6mCKSS1POh5TN1tuSdgtLxcnsjG6iVTr"
#define ANDROID_IDENTITY                                                      \
    "MEwDAgcAAgEgAiBK4dcDZUSLCxmvRfMWMAQf1JzSrLzZakLqDsULzT28OwIhAILbBS66JoN" \
    "1Xo2YsC1xDPDhukJjVO2guoeL+AM27Vfn"

// ルーチン
void routine(const char *in)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *sha1 = EVP_MD_fetch(NULL, "SHA-1", NULL);
#else
    const EVP_MD *sha1 = EVP_sha1();
#endif
    EVP_MD_CTX *ctx = NULL;
    unsigned char md[EVP_MAX_MD_SIZE];
    int i = 0;
    // 公開鍵長さ
    const size_t publickey_string_length = strlen(in);
    // 配列長さ
    const size_t input_buffer_size = publickey_string_length + IN2_SIZE;
    char input_buffer[input_buffer_size];
    char *verifier_head_ptr = NULL;
    uint64_t verifier = 0;
    size_t verifierLength = 0;
    int c = -1;
    int c_max = INT_MIN;
#pragma omp parallel private(ctx, md, i, input_buffer, verifier_head_ptr,     \
                             verifierLength, c)
    {
        ctx = EVP_MD_CTX_new();
        // ゼロ埋め初期化
        memset(input_buffer, 0, input_buffer_size);
        // バッファに公開鍵をコピー
        memcpy(input_buffer, in, publickey_string_length);
        verifier_head_ptr = input_buffer + publickey_string_length;
        // 0x01000000000を8スレ->2.5h,12スレ->1.67h(100min)->2.07h
        // 0x10000000000
#pragma omp for
        for (verifier = 0x01000000000UL; verifier < 0x02000000000UL;
             verifier++)
        {
            // 公開鍵の末尾にverifierを書き込み
            verifierLength
                = snprintf(verifier_head_ptr, IN2_SIZE, "%" PRIu64, verifier);
            // SHA1でハッシュを作成
            EVP_DigestInit(ctx, sha1);
            EVP_DigestUpdate(ctx, input_buffer,
                             publickey_string_length + verifierLength);
            EVP_DigestFinal(ctx, md, NULL);
            // if (memcmp(md, "\0\0\0\0\0", 3) == 0)
            c = __builtin_ctzl(le64toh(*(unsigned long *)md));
#pragma omp critical
            if (c_max < c)
            {
                printf("update!:%2d -> %2d(%" PRIu64 ")\n", c_max, c,
                       verifier);
                for (i = 0; i < SHA_DIGEST_LENGTH; i++)
                {
                    printf("%02x", md[i]);
                }
                printf("\n");
                c_max = c;
            }
            if (c >= 40)
            {
#pragma omp critical
                {
                    printf(_("verifier : %" PRIu64 "\n"), verifier);
                    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
                    {
                        printf("%02x", md[i]);
                    }
                    printf("\n");
                }
            }
        }
        EVP_MD_CTX_free(ctx);
        ctx = NULL;
    }
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(sha1);
#else
    // Do nothing because EVP_MD is const
#endif
}

/**
 *
 */
int main(const int argc, const char *argv[])
{
    const char *publicKey = (argc >= 2) ? argv[1] : DEFAULT_IDENTITY;
    time_t start = 0;
    time_t finish = 0;
    struct tm tm = { 0 };
    char timebuf[512] = "";

    start = time(NULL);
    localtime_r(&start, &tm);
    strftime(timebuf, 512, "%Y/%m/%d %T", &tm);
    printf("開始: %s\n", timebuf);
    routine(ANDROID_IDENTITY);
    finish = time(NULL);
    localtime_r(&finish, &tm);
    strftime(timebuf, 512, "%Y/%m/%d %T", &tm);
    printf("終わり！: %s(%" PRId64 ")\n", timebuf,
           (int64_t)difftime(finish, start));
    return EXIT_SUCCESS;
}
