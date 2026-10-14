#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/decoder.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define DEFAULT_HASH "sha256"

// --saltlen の特殊値 (OpenSSLの rsa_pss_saltlen に対応)
#define SALT_DIGEST (-1)   // ダイジェスト長 (既定)
#define SALT_MAX    (-2)   // 入る最大の長さ (emLen - hLen - 2)
#define SALT_AUTO   (-3)   // verify専用: EMから検出する

typedef struct {
    const EVP_MD *md;       // mHash と H に使うハッシュ
    const EVP_MD *mgf1_md;  // MGF1 に使うハッシュ
    int salt_len;           // 0以上の値 または SALT_*
} pss_params;

// MGF1 (Mask Generation Function 1) の実装
int mgf1(const EVP_MD *md, const unsigned char *seed, size_t seed_len, unsigned char *mask, size_t mask_len) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    size_t h_len = EVP_MD_get_size(md);
    uint32_t counter = 0;
    size_t out_len = 0;
    unsigned char cnt_buf[4];
    unsigned char h_buf[EVP_MAX_MD_SIZE];

    while (out_len < mask_len) {
        // カウンタをビッグエンディアンの4バイトに変換
        cnt_buf[0] = (counter >> 24) & 0xFF;
        cnt_buf[1] = (counter >> 16) & 0xFF;
        cnt_buf[2] = (counter >> 8)  & 0xFF;
        cnt_buf[3] = counter         & 0xFF;

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
        counter++;
    }

    EVP_MD_CTX_free(ctx);
    return 0;
}

// 汎用データ入力関数 (ファイル または stdin)
unsigned char* load_input(const char* filepath, size_t* out_len) {
    FILE* fp = stdin;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "rb");
        if (!fp) { perror("入力データを開けません"); return NULL; }
    } else {
#ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
#endif
    }

    size_t capacity = 1024 * 1024; // 初期 1MB
    unsigned char* buffer = malloc(capacity);
    size_t total_len = 0;

    size_t bytes_read;
    while ((bytes_read = fread(buffer + total_len, 1, 4096, fp)) > 0) {
        total_len += bytes_read;
        if (total_len + 4096 > capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
        }
    }

    if (fp != stdin) fclose(fp);
    *out_len = total_len;
    return buffer;
}

// 汎用データ出力関数 (ファイル または stdout)
void save_output(const char* filepath, const unsigned char* data, size_t len) {
    FILE* fp = stdout;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "wb");
        if (!fp) { perror("出力ファイルを開けません"); exit(1); }
    } else {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    }

    fwrite(data, 1, len, fp);
    if (fp != stdout) fclose(fp);
    else fflush(stdout);
}

// 鍵ファイル(PEM/DER、公開鍵/秘密鍵、SPKI/PKCS#1)からモジュラスのビット長を得る
int get_key_bits(const char *key_path, const char *inform) {
    size_t len = 0;
    unsigned char *buf = load_input(key_path, &len);
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
    int bits = EVP_PKEY_get_bits(pkey);
    EVP_PKEY_free(pkey);
    return bits;
}

// M' = (0x00)*8 || mHash || salt のハッシュを h に書く
static int pss_hash_mprime(const EVP_MD *md, const unsigned char *m_hash, size_t h_len,
                           const unsigned char *salt, size_t salt_len, unsigned char *h) {
    static const unsigned char zero8[8] = {0};
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    int ok = ctx &&
        EVP_DigestInit_ex(ctx, md, NULL) &&
        EVP_DigestUpdate(ctx, zero8, 8) &&
        EVP_DigestUpdate(ctx, m_hash, h_len) &&
        (salt_len == 0 || EVP_DigestUpdate(ctx, salt, salt_len)) &&
        EVP_DigestFinal_ex(ctx, h, NULL);
    EVP_MD_CTX_free(ctx);
    return ok ? 0 : -1;
}

// EMSA-PSS 署名用エンコード (署名生成時)
int emsa_pss_encode(const unsigned char *m_hash, const pss_params *pp, unsigned char *em, size_t em_len, int key_bits) {
    size_t h_len = EVP_MD_get_size(pp->md);

    // salt長の確定
    size_t salt_len;
    if (pp->salt_len == SALT_DIGEST) {
        salt_len = h_len;
    } else if (pp->salt_len == SALT_MAX) {
        if (em_len < h_len + 2) { fprintf(stderr, "エラー: 鍵サイズが小さすぎます。\n"); return -1; }
        salt_len = em_len - h_len - 2;
    } else if (pp->salt_len >= 0) {
        salt_len = (size_t)pp->salt_len;
    } else {
        fprintf(stderr, "エラー: --saltlen auto は verify でのみ使えます。\n");
        return -1;
    }

    // 境界チェック
    if (em_len < h_len + 2 || em_len - h_len - 2 < salt_len) {
        fprintf(stderr, "エラー: 鍵サイズが小さすぎます (emLen=%zu, 必要 %zu 以上)。\n", em_len, h_len + salt_len + 2);
        return -1;
    }

    // 1. Salt の生成 (OpenSSLのCSPRNGを使用)
    unsigned char *salt = malloc(salt_len ? salt_len : 1);
    if (!salt) return -1;
    if (salt_len && RAND_bytes(salt, (int)salt_len) != 1) {
        fprintf(stderr, "エラー: 暗号論的に安全な乱数の生成に失敗しました。\n");
        free(salt);
        return -1;
    }

    // 2. H = Hash(M')。EM内の配置予定場所に直接書き込む
    unsigned char *h = em + (em_len - h_len - 1);
    if (pss_hash_mprime(pp->md, m_hash, h_len, salt, salt_len, h) < 0) { free(salt); return -1; }

    // 3. DB = PS || 0x01 || salt の作成
    size_t db_len = em_len - h_len - 1;
    unsigned char *db = malloc(db_len);
    unsigned char *db_mask = malloc(db_len);
    if (!db || !db_mask) { free(salt); free(db); free(db_mask); return -1; }
    size_t ps_len = db_len - salt_len - 1;

    memset(db, 0, ps_len);
    db[ps_len] = 0x01;
    if (salt_len) memcpy(db + ps_len + 1, salt, salt_len);

    // 4. dbMask = MGF1(H, emLen - hLen - 1)
    if (mgf1(pp->mgf1_md, h, h_len, db_mask, db_len) < 0) {
        free(salt); free(db); free(db_mask);
        return -1;
    }

    // 5. maskedDB = DB xor dbMask
    for (size_t i = 0; i < db_len; i++) {
        em[i] = db[i] ^ db_mask[i];
    }

    // 6. 最上位ビットのクリア
    // emBits = key_bits - 1 に合わせて、先頭バイトの余剰ビット (8*emLen - emBits 個) を確実に0にする
    size_t em_bits = key_bits - 1;
    em[0] &= 0xFF >> (8 * em_len - em_bits);

    // 7. 最終識別子のセット
    em[em_len - 1] = 0xBC;

    free(salt);
    free(db);
    free(db_mask);
    return 0;
}

// EMSA-PSS 検証用デコード (署名検証時)
int emsa_pss_verify(const unsigned char *m_hash, const unsigned char *em, const pss_params *pp, size_t em_len, int key_bits) {
    size_t h_len = EVP_MD_get_size(pp->md);

    // 基準チェック
    if (em_len < h_len + 2 || em[em_len - 1] != 0xBC) return -1;
    size_t db_len = em_len - h_len - 1;

    // 最上位の余剰ビットチェック (encodeと同じく emBits = key_bits - 1 から求める)
    size_t em_bits = key_bits - 1;
    unsigned char top_mask = 0xFF >> (8 * em_len - em_bits);
    if ((em[0] & ~top_mask) != 0) return -1;

    // 1. maskedDB と H の抽出
    const unsigned char *masked_db = em;
    const unsigned char *h = em + db_len;

    // 2. dbMask = MGF1(H, dbLen)
    unsigned char *db_mask = malloc(db_len);
    unsigned char *db = malloc(db_len);
    if (!db_mask || !db) { free(db_mask); free(db); return -1; }
    if (mgf1(pp->mgf1_md, h, h_len, db_mask, db_len) < 0) {
        free(db_mask); free(db); return -1;
    }

    // 3. DB = maskedDB xor dbMask
    for (size_t i = 0; i < db_len; i++) {
        db[i] = masked_db[i] ^ db_mask[i];
    }

    // エンコード時と同様に、最上位の余剰ビットを落とす
    db[0] &= top_mask;

    // 4. パディングの正当性検証 (PS || 0x01) と salt 長の確定
    size_t ps_len;
    if (pp->salt_len == SALT_AUTO) {
        ps_len = 0;
        while (ps_len < db_len && db[ps_len] == 0x00) ps_len++;
    } else {
        size_t salt_len;
        if (pp->salt_len == SALT_DIGEST) salt_len = h_len;
        else if (pp->salt_len == SALT_MAX) salt_len = db_len - 1; // emLen - hLen - 2 (PSの長さが0)
        else salt_len = (size_t)pp->salt_len;
        if (db_len < salt_len + 1) { free(db_mask); free(db); return -1; }
        ps_len = db_len - salt_len - 1;
        for (size_t i = 0; i < ps_len; i++) {
            if (db[i] != 0x00) { free(db_mask); free(db); return -1; }
        }
    }
    if (ps_len >= db_len || db[ps_len] != 0x01) { free(db_mask); free(db); return -1; }

    // 5. Salt の抽出
    const unsigned char *salt = db + ps_len + 1;
    size_t salt_len = db_len - ps_len - 1;

    // 6. H' の再計算
    unsigned char h_prime[EVP_MAX_MD_SIZE];
    if (pss_hash_mprime(pp->md, m_hash, h_len, salt, salt_len, h_prime) < 0) {
        free(db_mask); free(db);
        return -1;
    }

    // 7. H と H' の比較
    int result = (CRYPTO_memcmp(h, h_prime, h_len) == 0) ? 0 : -1;

    free(db_mask);
    free(db);
    return result;
}

static const EVP_MD *lookup_digest(const char *name) {
    const EVP_MD *md = EVP_get_digestbyname(name);
    if (!md || EVP_MD_get_size(md) <= 0 || (EVP_MD_get_flags(md) & EVP_MD_FLAG_XOF)) {
        fprintf(stderr, "エラー: 使用できないハッシュ名です: %s (例: sha1, sha256, sha384, sha512, sha3-256)\n", name);
        return NULL;
    }
    return md;
}

static int parse_int(const char *s, int min, int *out) {
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno || end == s || *end != '\0' || v < min || v > INT32_MAX) return -1;
    *out = (int)v;
    return 0;
}

void print_usage(const char *prog) {
    printf("【EMSA-PSS パディング専用ツール】(RFC 8017 9.1)\n");
    printf("使用法:\n");
    printf("  %s <encode|verify> <input_msg> <output_em/input_em> [オプション]\n\n", prog);
    printf("  encode: メッセージ -> EM。RSA演算は含まない。\n");
    printf("  verify: メッセージとEMを突き合わせる。EMが短い場合は先頭を0x00で埋める\n");
    printf("          (rsa-calc の出力は先頭の0x00が落ちているため)。\n\n");
    printf("鍵長 (どちらか必須。emBits = ビット長 - 1 からEMの長さを求める):\n");
    printf("  --bits <数値>      鍵のビット長を直接指定します (例: 2048, 2097152)。\n");
    printf("  --key <ファイル>   RSA鍵ファイルからビット長を自動取得します (公開鍵/秘密鍵、SPKI/PKCS#1)。\n");
    printf("  --inform <auto|pem|der> 鍵ファイルの形式を指定します (デフォルト: auto)。\n\n");
    printf("PSSパラメータ (opensslの pkeyutl のオプションに対応):\n");
    printf("  --pss-hash <名前>  PSSのハッシュ。mHash と H = Hash(M') に使う (デフォルト: %s)。 [digest]\n", DEFAULT_HASH);
    printf("  --mgf1-hash <名前> MGF1のハッシュ (デフォルト: --pss-hash と同じ)。 [rsa_mgf1_md]\n");
    printf("  --saltlen <値>     salt長。数値 | digest(ダイジェスト長、デフォルト) | max |\n");
    printf("                     auto(verify専用: EMから検出)。 [rsa_pss_saltlen]\n");
    printf("  --prehashed        入力をメッセージではなく、--pss-hash で計算済みのダイジェスト\n");
    printf("                     (バイナリ)として扱います。長さがダイジェスト長と一致する必要があります。\n\n");
    printf("※ ファイルパスに '-' を指定すると標準入出力になります。\n");
    printf("終了コード: 0=成功, 1=引数/入出力エラー, 2=verify失敗\n");
}

int main(int argc, char *argv[]) {
    if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    char *mode = argv[1];
    char *msg_path = argv[2];
    char *em_path = argv[3];

    int key_bits = 0;
    char *key_path = NULL;
    char *inform = "auto";
    const char *hash_name = DEFAULT_HASH;
    const char *mgf1_name = NULL;
    int salt_len = SALT_DIGEST;
    int prehashed = 0;

    // オプション解析のループ
    for (int i = 4; i < argc; i++) {
        int has_val = (i + 1 < argc);
        if (strcmp(argv[i], "--bits") == 0 && has_val) {
            if (parse_int(argv[++i], 1, &key_bits) < 0) {
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
        } else if ((strcmp(argv[i], "--pss-hash") == 0 || strcmp(argv[i], "--hash") == 0) && has_val) {
            hash_name = argv[++i];
        } else if (strcmp(argv[i], "--mgf1-hash") == 0 && has_val) {
            mgf1_name = argv[++i];
        } else if (strcmp(argv[i], "--saltlen") == 0 && has_val) {
            const char *v = argv[++i];
            if (strcmp(v, "digest") == 0) salt_len = SALT_DIGEST;
            else if (strcmp(v, "max") == 0) salt_len = SALT_MAX;
            else if (strcmp(v, "auto") == 0) salt_len = SALT_AUTO;
            else if (parse_int(v, 0, &salt_len) < 0) {
                fprintf(stderr, "エラー: --saltlen には 0以上の整数 | digest | max | auto を指定してください: %s\n", v);
                return 1;
            }
        } else if (strcmp(argv[i], "--prehashed") == 0) {
            prehashed = 1;
        } else {
            fprintf(stderr, "エラー: 不明なオプション、または引数が不足しています: %s\n", argv[i]);
            return 1;
        }
    }

    if (strcmp(mode, "encode") != 0 && strcmp(mode, "verify") != 0) {
        fprintf(stderr, "未知のモード: %s (encode / verify)\n", mode);
        return 1;
    }
    if (key_path && key_bits > 0) {
        fprintf(stderr, "エラー: --bits と --key は同時に指定できません。\n");
        return 1;
    }

    pss_params pp;
    pp.md = lookup_digest(hash_name);
    if (!pp.md) return 1;
    pp.mgf1_md = mgf1_name ? lookup_digest(mgf1_name) : pp.md;
    if (!pp.mgf1_md) return 1;
    pp.salt_len = salt_len;

    // ビット数の決定
    if (key_path) {
        key_bits = get_key_bits(key_path, inform);
        if (key_bits < 0) {
            return 1; // 鍵読み込みエラー
        }
        fprintf(stderr, "鍵情報: %d bits として検出しました。\n", key_bits);
    }

    if (key_bits <= 0) {
        fprintf(stderr, "エラー: --bits または --key オプションで有効なビット長を指定してください。\n");
        return 1;
    }

    // ビット長からバイトサイズを計算 (切り上げ)
    size_t em_len = ((key_bits - 1) + 7) / 8;
    size_t h_len = EVP_MD_get_size(pp.md);

    // 1. 元メッセージのハッシュ化 (--prehashed のときは入力がそのままダイジェスト)
    size_t msg_len;
    unsigned char *msg_bytes = load_input(msg_path, &msg_len);
    if (!msg_bytes) return 1;

    unsigned char m_hash[EVP_MAX_MD_SIZE];
    if (prehashed) {
        if (msg_len != h_len) {
            fprintf(stderr, "エラー: --prehashed の入力は %s のダイジェスト長 (%zu バイト) である必要があります (実際は %zu バイト)。\n",
                    hash_name, h_len, msg_len);
            free(msg_bytes);
            return 1;
        }
        memcpy(m_hash, msg_bytes, h_len);
    } else {
        unsigned int md_len;
        if (!EVP_Digest(msg_bytes, msg_len, m_hash, &md_len, pp.md, NULL)) {
            fprintf(stderr, "エラー: ハッシュ計算に失敗しました。\n");
            free(msg_bytes);
            return 1;
        }
    }
    free(msg_bytes);

    if (strcmp(mode, "encode") == 0) {
        unsigned char *em = calloc(1, em_len);
        if (!em) return 1;
        if (emsa_pss_encode(m_hash, &pp, em, em_len, key_bits) < 0) {
            fprintf(stderr, "エンコード失敗\n");
            free(em); return 1;
        }
        save_output(em_path, em, em_len);
        free(em);
        return 0;
    }

    // verify
    size_t loaded_em_len;
    unsigned char *em_bytes = load_input(em_path, &loaded_em_len);
    if (!em_bytes) return 1;

    // emLen を超える先頭の0x00は取り除き、足りない分は0x00で埋める (I2OSP)
    const unsigned char *src = em_bytes;
    while (loaded_em_len > em_len && src[0] == 0x00) { src++; loaded_em_len--; }
    if (loaded_em_len > em_len) {
        fprintf(stderr, "エラー: パディングデータサイズが不正です (%zu バイト)。%zu バイト以下である必要があります。\n", loaded_em_len, em_len);
        free(em_bytes);
        return 1;
    }
    unsigned char *em = calloc(1, em_len);
    if (!em) { free(em_bytes); return 1; }
    if (loaded_em_len < em_len) {
        fprintf(stderr, "注意: EMが %zu バイトなので、先頭に %zu バイトの0x00を補います。\n", loaded_em_len, em_len - loaded_em_len);
    }
    memcpy(em + (em_len - loaded_em_len), src, loaded_em_len);
    free(em_bytes);

    int rc;
    if (emsa_pss_verify(m_hash, em, &pp, em_len, key_bits) == 0) {
        fprintf(stderr, "🟢 EMSA-PSS パディング検証成功 (%d bits)\n", key_bits);
        rc = 0; // 成功
    } else {
        fprintf(stderr, "🔴 EMSA-PSS パディング検証失敗\n");
        rc = 2; // 失敗コード
    }
    free(em);
    return rc;
}
