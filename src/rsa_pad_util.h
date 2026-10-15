#ifndef RSA_PAD_UTIL_H
#define RSA_PAD_UTIL_H

// eme-oaep / emsa-pss が共有する小さなヘルパー。

#include <stddef.h>
#include <openssl/evp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ファイル (または "-" で標準入力) を全部読む。失敗したら NULL。free() で解放する。 */
unsigned char *rsapad_load_input(const char *filepath, size_t *out_len);

/** ファイル (または "-" で標準出力) に書く。書き込みも閉じる処理も検査し、失敗したら -1。 */
int rsapad_save_output(const char *filepath, const unsigned char *data, size_t len);

/** MGF1 (RFC 8017 B.2.1)。mask に mask_len バイト書く。失敗したら -1。 */
int rsapad_mgf1(const EVP_MD *md, const unsigned char *seed, size_t seed_len,
                unsigned char *mask, size_t mask_len);

/**
 * RSA鍵ファイル (PEM/DER、公開鍵/秘密鍵、SPKI/PKCS#1) からモジュラスのビット長を返す。
 * inform は "auto" "pem" "der"。失敗したら -1。
 */
int rsapad_get_key_bits(const char *key_path, const char *inform);

/** ハッシュ名からEVP_MDを得る。未知の名前、XOF (shake) は拒否して NULL (理由をstderrに出す)。 */
const EVP_MD *rsapad_lookup_digest(const char *name);

/** 10進の整数を読む。min 以上 INT32_MAX 以下でなければ -1。 */
int rsapad_parse_int(const char *s, int min, int *out);

#ifdef __cplusplus
}
#endif

#endif
