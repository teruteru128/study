#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define KEY_BITS 2097152                 // 2,097,152 ビット
#define EMBYTES (KEY_BITS / 8)          // 262,144 バイト (256 KB)
#define SALT_LEN 32                      // Salt長 (SHA-256の出力サイズに合わせるのが一般的)

// MGF1 (Mask Generation Function 1) の実装
int mgf1(const EVP_MD *md, const unsigned char *seed, size_t seed_len, unsigned char *mask, size_t mask_len) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return -1;

    size_t h_len = EVP_MD_size(md);
    uint32_t counter = 0;
    size_t out_len = 0;
    unsigned char cnt_buf[4];
    unsigned char *h_buf = malloc(h_len);

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
            free(h_buf);
            EVP_MD_CTX_free(ctx);
            return -1;
        }

        size_t to_copy = (mask_len - out_len < h_len) ? (mask_len - out_len) : h_len;
        memcpy(mask + out_len, h_buf, to_copy);
        out_len += to_copy;
        counter++;
    }

    free(h_buf);
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

// EMSA-PSS 署名用エンコード (署名生成時)
int emsa_pss_encode(const unsigned char *m_hash, const EVP_MD *md, unsigned char *em) {
    size_t h_len = EVP_MD_size(md);
    
    // 巨大鍵空間に対する例外チェック
    if (EMBYTES < h_len + SALT_LEN + 2) {
        fprintf(stderr, "エラー: 鍵サイズが小さすぎます。\n");
        return -1;
    }

    // 1. Salt の生成 (本番運用では /dev/urandom 等の安全な乱数を使用)
    unsigned char salt[SALT_LEN];
    for (int i = 0; i < SALT_LEN; i++) salt[i] = rand() & 0xFF; 

    // 2. M' = (0x00)*8 || mHash || salt の作成とハッシュ化
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char zero8[8] = {0};
    unsigned char *h = em + (EMBYTES - h_len - 1); // EM内の配置予定場所に直接書き込む

    if (!ctx ||
        !EVP_DigestInit_ex(ctx, md, NULL) ||
        !EVP_DigestUpdate(ctx, zero8, 8) ||
        !EVP_DigestUpdate(ctx, m_hash, h_len) ||
        !EVP_DigestUpdate(ctx, salt, SALT_LEN) ||
        !EVP_DigestFinal_ex(ctx, h, NULL)) {
        if (ctx) EVP_MD_CTX_free(ctx);
        return -1;
    }
    EVP_MD_CTX_free(ctx);

    // 3. DB = PS || 0x01 || salt の作成
    size_t db_len = EMBYTES - h_len - 1;
    unsigned char *db = malloc(db_len);
    size_t ps_len = db_len - SALT_LEN - 1;
    
    memset(db, 0, ps_len);
    db[ps_len] = 0x01;
    memcpy(db + ps_len + 1, salt, SALT_LEN);

    // 4. dbMask = MGF1(H, emLen - hLen - 1)
    unsigned char *db_mask = malloc(db_len);
    if (mgf1(md, h, h_len, db_mask, db_len) < 0) {
        free(db); free(db_mask);
        return -1;
    }

    // 5. maskedDB = DB xor dbMask
    for (size_t i = 0; i < db_len; i++) {
        em[i] = db[i] ^ db_mask[i];
    }

    // 6. 最上位ビットのクリア (RSA鍵のビット長制限に合わせる。200万ジャストなら最上位バイトを部分クリア)
    // 今回は200万ビット(バイト境界ジャスト)なので、最上位ビットのみクリアを厳密に行う場合は以下
    em[0] &= 0x7F; 

    // 7. 最終識別子のセット
    em[EMBYTES - 1] = 0xBC;

    free(db);
    free(db_mask);
    return 0;
}

// EMSA-PSS 検証用デコード (署名検証時)
int emsa_pss_verify(const unsigned char *m_hash, const unsigned char *em, const EVP_MD *md) {
    size_t h_len = EVP_MD_size(md);
    size_t db_len = EMBYTES - h_len - 1;

    // 基準チェック
    if (EMBYTES < h_len + SALT_LEN + 2 || em[EMBYTES - 1] != 0xBC) return -1;
    if ((em[0] & 0x80) != 0) return -1; // 最上位ビットチェック

    // 1. maskedDB と H の抽出
    const unsigned char *masked_db = em;
    const unsigned char *h = em + db_len;

    // 2. dbMask = MGF1(H, dbLen)
    unsigned char *db_mask = malloc(db_len);
    if (mgf1(md, h, h_len, db_mask, db_len) < 0) {
        free(db_mask); return -1;
    }

    // 3. DB = maskedDB xor dbMask
    unsigned char *db = malloc(db_len);
    for (size_t i = 0; i < db_len; i++) {
        db[i] = masked_db[i] ^ db_mask[i];
    }
    db[0] &= 0x7F; // エンコード時と同様のマスク解除

    // 4. パディングの正当性検証 (PS || 0x01)
    size_t ps_len = db_len - SALT_LEN - 1;
    for (size_t i = 0; i < ps_len; i++) {
        if (db[i] != 0x00) { free(db_mask); free(db); return -1; }
    }
    if (db[ps_len] != 0x01) { free(db_mask); free(db); return -1; }

    // 5. Salt の抽出
    unsigned char *salt = db + ps_len + 1;

    // 6. H' の再計算
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    unsigned char zero8[8] = {0};
    unsigned char h_prime[EVP_MAX_MD_SIZE];

    if (!ctx ||
        !EVP_DigestInit_ex(ctx, md, NULL) ||
        !EVP_DigestUpdate(ctx, zero8, 8) ||
        !EVP_DigestUpdate(ctx, m_hash, h_len) ||
        !EVP_DigestUpdate(ctx, salt, SALT_LEN) ||
        !EVP_DigestFinal_ex(ctx, h_prime, NULL)) {
        if (ctx) EVP_MD_CTX_free(ctx);
        free(db_mask); free(db);
        return -1;
    }
    EVP_MD_CTX_free(ctx);

    // 7. H と H' の比較
    int result = (memcmp(h, h_prime, h_len) == 0) ? 0 : -1;

    free(db_mask);
    free(db);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("【200万bit対応 EMSA-PSS パディング専用ツール】\n");
        printf("使用法:\n");
        printf("  エンコード (署名前): %s encode <input_msg> <output_em>\n", argv[0]);
        printf("  デコード   (検証前): %s verify <input_msg> <input_em>\n", argv[0]);
        printf("  ※ ファイルパスに '-' を指定すると標準入出力（パイプ）になります。\n");
        return 1;
    }

    char *mode = argv[1];
    char *msg_path = argv[2];
    char *em_path = argv[3];

    // 1. 元メッセージのハッシュ化 (SHA-256)
    size_t msg_len;
    unsigned char *msg_bytes = load_input(msg_path, &msg_len);
    if (!msg_bytes) return 1;

    const EVP_MD *md = EVP_sha256();
    unsigned char m_hash[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    
    EVP_Digest(msg_bytes, msg_len, m_hash, &md_len, md, NULL);
    free(msg_bytes);

    if (strcmp(mode, "encode") == 0) {
        unsigned char *em = calloc(1, EMBYTES);
        if (emsa_pss_encode(m_hash, md, em) < 0) {
            fprintf(stderr, "エンコード失敗\n");
            free(em); return 1;
        }
        save_output(em_path, em, EMBYTES);
        free(em);
        
    } else if (strcmp(mode, "verify") == 0) {
        size_t em_len;
        unsigned char *em_bytes = load_input(em_path, &em_len);
        if (!em_bytes || em_len != EMBYTES) {
            fprintf(stderr, "エラー: パディングデータサイズが不正です (%zu バイト)。%d バイトである必要があります。\n", em_len, EMBYTES);
            if (em_bytes) free(em_bytes);
            return 1;
        }

        if (emsa_pss_verify(m_hash, em_bytes, md) == 0) {
            // 計算層に渡すため、検証成功時は「パディングが正常に剥がれた状態のハッシュ値」をそのままパイプ等で出力可能にする
            // または、呼び出し元への終了ステータスで成否を伝える
            fprintf(stderr, "🟢 EMSA-PSS パディング検証成功\n");
            free(em_bytes);
            return 0; // 成功
        } else {
            fprintf(stderr, "🔴 EMSA-PSS パディング検証失敗\n");
            free(em_bytes);
            return 2; // 失敗コード
        }
    } else {
        fprintf(stderr, "未知のモード: %s\n", mode);
        return 1;
    }

    return 0;
}
