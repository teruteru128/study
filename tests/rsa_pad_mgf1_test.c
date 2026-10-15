// rsapad_mgf1 (src/rsa_pad_util.c) の単体テスト。成功なら0、失敗なら1を返す。
//  1. 既知の値 (KAT): MGF1-SHA1 の解説でよく引かれる値。OpenSSL の出力とも一致することを確認してある
//  2. OpenSSL の PKCS1_MGF1 との突き合わせ。ハッシュ × シードの長さ × マスクの長さの全組み合わせ。
//     PKCS1_MGF1 は OpenSSL 3 で非推奨だが、検算の基準としては最適なのでここでだけ使う。
#define OPENSSL_SUPPRESS_DEPRECATED
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/rsa_pad_util.h"

static int failures;
static int checks;

static void fail(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#include <stdarg.h>
static void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    failures++;
}

static int unhex(const char *hex, unsigned char *out)
{
    size_t n = strlen(hex) / 2;
    for (size_t i = 0; i < n; i++) {
        unsigned v;
        sscanf(hex + 2 * i, "%2x", &v);
        out[i] = (unsigned char)v;
    }
    return (int)n;
}

static void kat(const char *seed, const char *expected_hex)
{
    unsigned char want[128], got[128];
    int len = unhex(expected_hex, want);
    checks++;
    if (rsapad_mgf1(EVP_sha1(), (const unsigned char *)seed, strlen(seed), got, len) != 0 ||
        memcmp(got, want, len) != 0)
        fail("KAT 不一致: MGF1-SHA1(\"%s\", %d)", seed, len);
}

int main(void)
{
    kat("foo", "1ac907");
    kat("foo", "1ac9075cd4");
    kat("bar", "bc0c655e01");
    kat("bar", "bc0c655e016bc2931d85a2e675181adcef7f581f76df2739da74faac41627be2f7f415c89e983fd0ce80ced9878641cb4876");

    static const char *names[] = {"sha1", "sha224", "sha256", "sha384", "sha512", "sha3-256", "sha3-512"};
    static const size_t seed_lens[] = {0, 1, 20, 32, 64, 65, 200};
    static const size_t extra_mask_lens[] = {0, 1, 1000, 4097, 262144};
    unsigned char seed[256];
    for (size_t i = 0; i < sizeof seed; i++) seed[i] = (unsigned char)(i * 37 + 11);

    unsigned char *a = malloc(262144 + 1), *b = malloc(262144 + 1);
    if (!a || !b) return 1;

    for (size_t n = 0; n < sizeof names / sizeof *names; n++) {
        const EVP_MD *md = EVP_get_digestbyname(names[n]);
        if (!md) { fail("ハッシュが無い: %s", names[n]); continue; }
        size_t h = (size_t)EVP_MD_get_size(md);
        // hLen の前後 (境界) と倍数、それに大きな値
        size_t mask_lens[5 + 7];
        size_t nm = 0;
        for (size_t k = 0; k < sizeof extra_mask_lens / sizeof *extra_mask_lens; k++) mask_lens[nm++] = extra_mask_lens[k];
        mask_lens[nm++] = h - 1;
        mask_lens[nm++] = h;
        mask_lens[nm++] = h + 1;
        mask_lens[nm++] = 2 * h - 1;
        mask_lens[nm++] = 2 * h;
        mask_lens[nm++] = 2 * h + 1;
        mask_lens[nm++] = 7 * h + 3;
        for (size_t s = 0; s < sizeof seed_lens / sizeof *seed_lens; s++) {
            for (size_t m = 0; m < nm; m++) {
                size_t len = mask_lens[m];
                memset(a, 0xAA, len + 1);
                memset(b, 0xBB, len + 1);
                checks++;
                if (PKCS1_MGF1(b, (long)len, seed, (long)seed_lens[s], md) != 0) {
                    fail("PKCS1_MGF1 が失敗: %s seed=%zu mask=%zu", names[n], seed_lens[s], len);
                    continue;
                }
                if (rsapad_mgf1(md, seed, seed_lens[s], a, len) != 0) {
                    fail("rsapad_mgf1 が失敗: %s seed=%zu mask=%zu", names[n], seed_lens[s], len);
                    continue;
                }
                if (memcmp(a, b, len) != 0) fail("不一致: %s seed=%zu mask=%zu", names[n], seed_lens[s], len);
                // 指定した長さの先を書き潰していないこと
                if (a[len] != 0xAA) fail("範囲外に書いた: %s seed=%zu mask=%zu", names[n], seed_lens[s], len);
            }
        }
    }
    free(a);
    free(b);
    printf("rsa_pad_mgf1_test: %d 件中 %d 件失敗\n", checks, failures);
    return failures ? 1 : 0;
}
