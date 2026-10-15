// eme-oaep / emsa-pss が共有する小さなヘルパー (入出力、MGF1、鍵ファイルからのビット長取得など)
#include "rsa_pad_util.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/decoder.h>
#include <openssl/evp.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

// 汎用データ入力関数 (ファイル または stdin)
unsigned char *rsapad_load_input(const char *filepath, size_t *out_len)
{
    FILE *fp = stdin;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "rb");
        if (!fp) {
            fprintf(stderr, "入力を開けません: %s: %s\n", filepath, strerror(errno));
            return NULL;
        }
    } else {
#ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
#endif
    }

    size_t capacity = 1024 * 1024;
    size_t total_len = 0;
    unsigned char *buffer = malloc(capacity);
    if (!buffer) {
        if (fp != stdin) fclose(fp);
        return NULL;
    }

    size_t bytes_read;
    while ((bytes_read = fread(buffer + total_len, 1, 4096, fp)) > 0) {
        total_len += bytes_read;
        if (total_len + 4096 > capacity) {
            capacity *= 2;
            unsigned char *grown = realloc(buffer, capacity);
            if (!grown) {
                free(buffer);
                if (fp != stdin) fclose(fp);
                return NULL;
            }
            buffer = grown;
        }
    }

    int failed = ferror(fp);
    if (fp != stdin) fclose(fp);
    if (failed) {
        fprintf(stderr, "入力の読み込み中にエラーが発生しました: %s\n", filepath);
        free(buffer);
        return NULL;
    }
    *out_len = total_len;
    return buffer;
}

// 汎用データ出力関数 (ファイル または stdout)
int rsapad_save_output(const char *filepath, const unsigned char *data, size_t len)
{
    FILE *fp = stdout;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "wb");
        if (!fp) {
            fprintf(stderr, "出力ファイルを開けません: %s: %s\n", filepath, strerror(errno));
            return -1;
        }
    } else {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    }

    int ok = (fwrite(data, 1, len, fp) == len);
    if (fp != stdout) {
        if (fclose(fp) != 0) ok = 0;
    } else if (fflush(stdout) != 0) {
        ok = 0;
    }
    if (!ok) fprintf(stderr, "出力の書き込みに失敗しました: %s\n", filepath);
    return ok ? 0 : -1;
}

// MGF1 (RFC 8017 B.2.1)
int rsapad_mgf1(const EVP_MD *md, const unsigned char *seed, size_t seed_len,
                unsigned char *mask, size_t mask_len)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    size_t h_len = (size_t)EVP_MD_get_size(md);
    unsigned char h_buf[EVP_MAX_MD_SIZE];
    size_t out_len = 0;

    for (uint32_t counter = 0; out_len < mask_len; counter++) {
        unsigned char cnt_buf[4] = {
            (unsigned char)(counter >> 24), (unsigned char)(counter >> 16),
            (unsigned char)(counter >> 8),  (unsigned char)counter,
        };
        if (!EVP_DigestInit_ex(ctx, md, NULL) ||
            !EVP_DigestUpdate(ctx, seed, seed_len) ||
            !EVP_DigestUpdate(ctx, cnt_buf, 4) ||
            !EVP_DigestFinal_ex(ctx, h_buf, NULL)) {
            EVP_MD_CTX_free(ctx);
            return -1;
        }
        size_t to_copy = (mask_len - out_len < h_len) ? (mask_len - out_len) : h_len;
        memcpy(mask + out_len, h_buf, to_copy);
        out_len += to_copy;
    }

    OPENSSL_cleanse(h_buf, sizeof(h_buf));
    EVP_MD_CTX_free(ctx);
    return 0;
}

// 鍵ファイル(PEM/DER、公開鍵/秘密鍵、SPKI/PKCS#1)からモジュラスのビット長を得る
int rsapad_get_key_bits(const char *key_path, const char *inform)
{
    size_t len = 0;
    unsigned char *buf = rsapad_load_input(key_path, &len);
    if (!buf) return -1;

    EVP_PKEY *pkey = NULL;
    // 入力形式をNULLにするとPEM/DERを自動判別する。selection=0は公開鍵・秘密鍵どちらも許す
    OSSL_DECODER_CTX *dctx = OSSL_DECODER_CTX_new_for_pkey(
        &pkey, strcmp(inform, "auto") == 0 ? NULL : (strcmp(inform, "pem") == 0 ? "PEM" : "DER"),
        NULL, "RSA", 0, NULL, NULL);
    const unsigned char *p = buf;
    size_t remaining = len;
    int ok = dctx && OSSL_DECODER_from_data(dctx, &p, &remaining);
    OSSL_DECODER_CTX_free(dctx);
    free(buf);

    if (!ok || !pkey) {
        fprintf(stderr, "エラー: 鍵ファイルの解析に失敗しました。形式 (--inform) が正しいか確認してください。\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    if (EVP_PKEY_get_base_id(pkey) != EVP_PKEY_RSA && EVP_PKEY_get_base_id(pkey) != EVP_PKEY_RSA_PSS) {
        fprintf(stderr, "エラー: RSA鍵ではありません。\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    int bits = EVP_PKEY_get_bits(pkey);
    EVP_PKEY_free(pkey);
    return bits;
}

const EVP_MD *rsapad_lookup_digest(const char *name)
{
    const EVP_MD *md = EVP_get_digestbyname(name);
    if (!md || EVP_MD_get_size(md) <= 0 || (EVP_MD_get_flags(md) & EVP_MD_FLAG_XOF)) {
        fprintf(stderr, "エラー: 使用できないハッシュ名です: %s (例: sha1, sha256, sha384, sha512, sha3-256)\n", name);
        return NULL;
    }
    return md;
}

// 10進の整数を読む。min 以上 INT32_MAX 以下でなければ -1
int rsapad_parse_int(const char *s, int min, int *out)
{
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || end == s || *end != '\0' || v < min || v > INT32_MAX) return -1;
    *out = (int)v;
    return 0;
}
