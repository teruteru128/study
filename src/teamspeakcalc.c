
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

/**
 *
 */
int main(void)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD *sha1 = EVP_MD_fetch(NULL, "SHA-1", NULL);
#else
    const EVP_MD *sha1 = EVP_sha1();
#endif
    EVP_MD_CTX *ctx = NULL;
    unsigned char md[EVP_MAX_MD_SIZE];
    int i = 0;
    char in1[125] = "";
    uint64_t in1Length = 0;
    char *in2 = NULL;
    uint64_t verifier = 0;
    size_t verifierLength = 0;
    int c = -1;
    int c_max = INT_MIN;
    time_t start = time(NULL);
    time_t finish = 0;
    struct tm tm = { 0 };
    localtime_r(&start, &tm);
    char timebuf[512] = "";
    strftime(timebuf, 512, "%Y/%m/%d %T", &tm);

    printf("開始: %s\n", timebuf);
#pragma omp parallel private(ctx, md, i, in1, in1Length, in2, verifierLength, \
                             c)
    {
        ctx = EVP_MD_CTX_new();
        in1Length = strlen(DEFAULT_IDENTITY);
        memcpy(in1, DEFAULT_IDENTITY, in1Length);
        in2 = in1 + in1Length;
        // 0x01000000000
        // 0x10000000000
#pragma omp for
        for (verifier = 11241536114; verifier < 0x01000000000UL; verifier++)
        {
            EVP_DigestInit(ctx, sha1);
            verifierLength = snprintf(in2, IN2_SIZE, "%" PRIu64, verifier);
            EVP_DigestUpdate(ctx, in1, in1Length + verifierLength);
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

    finish = time(NULL);
    localtime_r(&finish, &tm);
    strftime(timebuf, 512, "%Y/%m/%d %T", &tm);
    printf("終わり！: %s(%ld)\n", timebuf, (int64_t)difftime(finish, start));
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MD_free(sha1);
#endif
    return EXIT_SUCCESS;
}
