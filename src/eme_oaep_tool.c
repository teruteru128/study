// EME-OAEP (RFC 8017 7.1.1 の EME-OAEP encoding) 専用ツール
//
//   EM = 0x00 || maskedSeed || maskedDB   (長さ k = RSA法の八ビット組長)
//   DB = lHash || PS || 0x01 || M
//
// RSA演算そのものは行わない。encodeで作ったEMを rsa-calc pub-pow に渡せば暗号文になり、
// rsa-calc pri-pow の出力をdecodeに渡せば平文に戻る。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include "rsa_pad_util.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define DEFAULT_HASH "sha256"

// EME-OAEP encoding。em は k バイト。
static int eme_oaep_encode(const unsigned char *msg, size_t msg_len,
                           const unsigned char *label, size_t label_len,
                           const EVP_MD *md, const EVP_MD *mgf1_md,
                           unsigned char *em, size_t k)
{
    size_t h_len = (size_t)EVP_MD_get_size(md);
    size_t seed_len = h_len;

    if (k < 2 * h_len + 2) {
        fprintf(stderr, "エラー: 鍵サイズが小さすぎます (k=%zu, 最低 %zu バイト必要)。\n", k, 2 * h_len + 2);
        return -1;
    }
    if (msg_len > k - 2 * h_len - 2) {
        fprintf(stderr, "エラー: メッセージが長すぎます (%zu バイト)。最大 %zu バイトです。\n",
                msg_len, k - 2 * h_len - 2);
        return -1;
    }

    size_t db_len = k - h_len - 1;
    unsigned char *seed = em + 1;
    unsigned char *db = em + 1 + h_len;
    int ret = -1;
    unsigned char *mask = malloc(db_len);
    if (!mask) return -1;

    // EM = 0x00 || seed || DB を em 上に直接組む。DB = lHash || PS || 0x01 || M
    em[0] = 0x00;
    if (!EVP_Digest(label, label_len, db, NULL, md, NULL)) goto done;
    memset(db + h_len, 0, db_len - h_len - msg_len - 1);
    db[db_len - msg_len - 1] = 0x01;
    if (msg_len) memcpy(db + db_len - msg_len, msg, msg_len);

    if (RAND_bytes(seed, (int)seed_len) != 1) {
        fprintf(stderr, "エラー: 暗号論的に安全な乱数の生成に失敗しました。\n");
        goto done;
    }

    // maskedDB = DB xor MGF(seed, dbLen)
    if (rsapad_mgf1(mgf1_md, seed, seed_len, mask, db_len) < 0) goto done;
    for (size_t i = 0; i < db_len; i++) db[i] ^= mask[i];

    // maskedSeed = seed xor MGF(maskedDB, hLen)
    if (rsapad_mgf1(mgf1_md, db, db_len, mask, seed_len) < 0) goto done;
    for (size_t i = 0; i < seed_len; i++) seed[i] ^= mask[i];

    ret = 0;
done:
    OPENSSL_cleanse(mask, db_len);
    free(mask);
    return ret;
}

// 定数時間のマスク演算 (全ビット1 or 0)
static size_t ct_is_zero(size_t x) { return (size_t)0 - (size_t)(((x | (0 - x)) >> (sizeof(size_t) * 8 - 1)) ^ 1); }
static size_t ct_eq(size_t a, size_t b) { return ct_is_zero(a ^ b); }

// EME-OAEP decoding。成功すると *out_msg に平文位置(em内)、*out_len に長さを返す。
// RFC 8017 の指示どおり、失敗理由は区別しない(Manger攻撃対策)。
static int eme_oaep_decode(unsigned char *em, size_t k,
                           const unsigned char *label, size_t label_len,
                           const EVP_MD *md, const EVP_MD *mgf1_md,
                           const unsigned char **out_msg, size_t *out_len)
{
    size_t h_len = (size_t)EVP_MD_get_size(md);
    if (k < 2 * h_len + 2) return -1;

    size_t db_len = k - h_len - 1;
    unsigned char *seed = em + 1;
    unsigned char *db = em + 1 + h_len;
    unsigned char l_hash[EVP_MAX_MD_SIZE];
    unsigned char *mask = malloc(db_len);
    if (!mask) return -1;

    int ret = -1;
    if (!EVP_Digest(label, label_len, l_hash, NULL, md, NULL)) goto done;

    // seed = maskedSeed xor MGF(maskedDB, hLen)、DB = maskedDB xor MGF(seed, dbLen)
    if (rsapad_mgf1(mgf1_md, db, db_len, mask, h_len) < 0) goto done;
    for (size_t i = 0; i < h_len; i++) seed[i] ^= mask[i];
    if (rsapad_mgf1(mgf1_md, seed, h_len, mask, db_len) < 0) goto done;
    for (size_t i = 0; i < db_len; i++) db[i] ^= mask[i];

    size_t good = ct_is_zero(em[0]);
    good &= ct_is_zero((size_t)CRYPTO_memcmp(db, l_hash, h_len));

    // PS (0x00の連続) の直後に0x01が来ることを確認する
    size_t looking = ~(size_t)0;
    size_t bad = 0;
    size_t one_index = 0;
    for (size_t i = h_len; i < db_len; i++) {
        size_t is_one = ct_eq(db[i], 1);
        size_t is_zero = ct_is_zero(db[i]);
        size_t found_now = looking & is_one;
        one_index = (found_now & (i + 1)) | (~found_now & one_index);
        bad |= looking & ~is_one & ~is_zero;
        looking &= ~is_one;
    }
    good &= ~looking & ~bad;

    if (good) {
        *out_msg = db + one_index;
        *out_len = db_len - one_index;
        ret = 0;
    }
done:
    OPENSSL_cleanse(mask, db_len);
    OPENSSL_cleanse(l_hash, sizeof(l_hash));
    free(mask);
    return ret;
}

static void print_usage(const char *prog)
{
    printf("【EME-OAEP パディング専用ツール】(RFC 8017 7.1.1)\n");
    printf("使用法:\n");
    printf("  %s <encode|decode> <input> <output> [オプション]\n\n", prog);
    printf("  encode: 平文 -> EM (鍵長と同じ長さ k バイト)。RSA演算は含まない。\n");
    printf("  decode: EM -> 平文。EMが k バイトより短い場合は先頭を0x00で埋める\n");
    printf("          (rsa-calc の出力は先頭の0x00が落ちているため)。\n\n");
    printf("鍵長 (どちらか必須。EMの長さは k = ceil(bits/8) バイトになる):\n");
    printf("  --bits <数値>        鍵のビット長を直接指定します (例: 2048, 2097152)。\n");
    printf("  --key <ファイル>     RSA鍵ファイルからビット長を自動取得します (公開鍵/秘密鍵、SPKI/PKCS#1)。\n");
    printf("  --inform <auto|pem|der> 鍵ファイルの形式 (デフォルト: auto)。\n\n");
    printf("OAEPパラメータ:\n");
    printf("  --oaep-hash <名前>   OAEPのハッシュ。ラベルのハッシュ lHash に使う (デフォルト: %s)。\n", DEFAULT_HASH);
    printf("  --mgf1-hash <名前>   MGF1のハッシュ (デフォルト: --oaep-hash と同じ)。\n");
    printf("  --label <文字列>     ラベル (デフォルト: 空)。\n");
    printf("  --label-file <ファイル> ラベルをファイルから読む ('-' で標準入力)。--labelとは併用不可。\n\n");
    printf("※ ファイルパスに '-' を指定すると標準入出力になります。\n");
    printf("終了コード: 0=成功, 1=引数/入出力エラー, 2=decode失敗\n");
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *in_path = argv[2];
    const char *out_path = argv[3];

    int key_bits = 0;
    const char *key_path = NULL;
    const char *inform = "auto";
    const char *hash_name = DEFAULT_HASH;
    const char *mgf1_name = NULL;
    const char *label_str = NULL;
    const char *label_path = NULL;

    for (int i = 4; i < argc; i++) {
        int has_val = (i + 1 < argc);
        if (strcmp(argv[i], "--bits") == 0 && has_val) {
            if (rsapad_parse_int(argv[++i], 1, &key_bits) < 0) {
                fprintf(stderr, "エラー: --bits には正の整数を指定してください: %s\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "--key") == 0 && has_val) {
            key_path = argv[++i];
        } else if (strcmp(argv[i], "--inform") == 0 && has_val) {
            inform = argv[++i];
            if (strcmp(inform, "pem") != 0 && strcmp(inform, "der") != 0 && strcmp(inform, "auto") != 0) {
                fprintf(stderr, "エラー: --inform は 'auto' 'pem' 'der' のいずれかを指定してください。\n");
                return 1;
            }
        } else if ((strcmp(argv[i], "--oaep-hash") == 0 || strcmp(argv[i], "--hash") == 0) && has_val) {
            hash_name = argv[++i];
        } else if (strcmp(argv[i], "--mgf1-hash") == 0 && has_val) {
            mgf1_name = argv[++i];
        } else if (strcmp(argv[i], "--label") == 0 && has_val) {
            label_str = argv[++i];
        } else if (strcmp(argv[i], "--label-file") == 0 && has_val) {
            label_path = argv[++i];
        } else {
            fprintf(stderr, "エラー: 不明なオプション、または引数が不足しています: %s\n", argv[i]);
            return 1;
        }
    }

    if (strcmp(mode, "encode") != 0 && strcmp(mode, "decode") != 0) {
        fprintf(stderr, "未知のモード: %s (encode / decode)\n", mode);
        return 1;
    }
    if (label_str && label_path) {
        fprintf(stderr, "エラー: --label と --label-file は同時に指定できません。\n");
        return 1;
    }
    if (key_path && key_bits > 0) {
        fprintf(stderr, "エラー: --bits と --key は同時に指定できません。\n");
        return 1;
    }
    if (label_path && strcmp(label_path, "-") == 0 && strcmp(in_path, "-") == 0) {
        fprintf(stderr, "エラー: 入力と --label-file の両方を標準入力にはできません。\n");
        return 1;
    }

    const EVP_MD *md = rsapad_lookup_digest(hash_name);
    if (!md) return 1;
    const EVP_MD *mgf1_md = mgf1_name ? rsapad_lookup_digest(mgf1_name) : md;
    if (!mgf1_md) return 1;

    if (key_path) {
        key_bits = rsapad_get_key_bits(key_path, inform);
        if (key_bits <= 0) return 1;
        fprintf(stderr, "鍵情報: %d bits として検出しました。\n", key_bits);
    }
    if (key_bits <= 0) {
        fprintf(stderr, "エラー: --bits または --key オプションで有効なビット長を指定してください。\n");
        return 1;
    }
    size_t k = ((size_t)key_bits + 7) / 8;

    // ラベル
    unsigned char *label = NULL;
    size_t label_len = 0;
    int label_allocated = 0;
    if (label_path) {
        label = rsapad_load_input(label_path, &label_len);
        if (!label) return 1;
        label_allocated = 1;
    } else if (label_str) {
        label = (unsigned char *)label_str;
        label_len = strlen(label_str);
    }
    static const unsigned char empty = 0;
    const unsigned char *label_ptr = label ? label : &empty;

    int rc = 1;
    size_t in_len = 0;
    unsigned char *in_buf = rsapad_load_input(in_path, &in_len);
    if (!in_buf) goto out;

    if (strcmp(mode, "encode") == 0) {
        unsigned char *em = calloc(1, k);
        if (!em) goto out_in;
        if (eme_oaep_encode(in_buf, in_len, label_ptr, label_len, md, mgf1_md, em, k) < 0) {
            fprintf(stderr, "エンコード失敗\n");
            free(em);
            goto out_in;
        }
        rc = rsapad_save_output(out_path, em, k) < 0 ? 1 : 0;
        free(em);
        if (rc == 0) fprintf(stderr, "🟢 EME-OAEP エンコード成功 (%d bits, EM %zu バイト, hash=%s, mgf1=%s)\n",
                             key_bits, k, hash_name, mgf1_name ? mgf1_name : hash_name);
    } else {
        if (in_len > k) {
            fprintf(stderr, "エラー: EMのサイズが不正です (%zu バイト)。%zu バイト以下である必要があります。\n", in_len, k);
            goto out_in;
        }
        unsigned char *em = calloc(1, k);
        if (!em) goto out_in;
        if (in_len < k) fprintf(stderr, "注意: EMが %zu バイトなので、先頭に %zu バイトの0x00を補います。\n", in_len, k - in_len);
        memcpy(em + (k - in_len), in_buf, in_len); // 左ゼロ埋め (I2OSP)

        const unsigned char *msg = NULL;
        size_t msg_len = 0;
        if (eme_oaep_decode(em, k, label_ptr, label_len, md, mgf1_md, &msg, &msg_len) == 0) {
            rc = rsapad_save_output(out_path, msg, msg_len) < 0 ? 1 : 0;
            if (rc == 0) fprintf(stderr, "🟢 EME-OAEP デコード成功 (平文 %zu バイト)\n", msg_len);
        } else {
            fprintf(stderr, "🔴 EME-OAEP デコード失敗\n");
            rc = 2;
        }
        OPENSSL_cleanse(em, k);
        free(em);
    }

out_in:
    OPENSSL_cleanse(in_buf, in_len);
    free(in_buf);
out:
    if (label_allocated) free(label);
    return rc;
}
