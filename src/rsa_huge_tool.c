#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <gmp.h>

// DERファイルをメモリに読み込む関数
unsigned char* load_der_file(const char* filepath, size_t* out_len) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) { perror("鍵ファイルを開けません"); return NULL; }
    fseek(fp, 0, SEEK_END); long fsize = ftell(fp); fseek(fp, 0, SEEK_SET);
    unsigned char* buffer = malloc(fsize);
    if (!buffer) { fclose(fp); return NULL; }
    fread(buffer, 1, fsize, fp); fclose(fp);
    *out_len = fsize; return buffer;
}

// PKCS#1 DERから巨大整数を抽出するパーサー（要素数をカウントして返す）
int parse_pkcs1_key(const unsigned char* der, size_t der_len, mpz_t* elements, int max_elements) {
    int count = 0; size_t i = 0;
    if (der[i] == 0x30) { i++; if (der[i] & 0x80) { i += (der[i] & 0x7F) + 1; } else { i++; } }
    while (i < der_len && count < max_elements) {
        if (der[i] == 0x02) {
            i++; size_t len = 0;
            if (der[i] & 0x80) {
                int len_bytes = der[i] & 0x7F; i++;
                for (int j = 0; j < len_bytes; j++) { len = (len << 8) | der[i++]; }
            } else { len = der[i++]; }
            mpz_import(elements[count], len, 1, 1, 1, 0, &der[i]);
            count++; i += len;
        } else { i++; }
    }
    return count;
}

// ファイルまたは標準入力から全データを読み込む
unsigned char* load_input_data(const char* filepath, size_t* out_len) {
    FILE* fp = stdin;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "rb");
        if (!fp) { perror("入力ファイルを開けません"); return NULL; }
    } else {
        // Windows環境などの場合は、標準入力をバイナリモードにする必要がある（Linux等では不要）
        #ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
        #endif
    }

    size_t capacity = 512 * 1024; // 初期バッファ: 512KB (200万bit = 250KBが収まるサイズ)
    unsigned char* buffer = malloc(capacity);
    size_t total_len = 0;

    if (fp == stdin) {
        // パイプ（ストリーム）からの読み込み
        size_t bytes_read;
        while ((bytes_read = fread(buffer + total_len, 1, 4096, fp)) > 0) {
            total_len += bytes_read;
            if (total_len + 4096 > capacity) {
                capacity *= 2;
                buffer = realloc(buffer, capacity);
            }
        }
    } else {
        // 通常ファイルからの高速読み込み
        fseek(fp, 0, SEEK_END); long fsize = ftell(fp); fseek(fp, 0, SEEK_SET);
        buffer = realloc(buffer, fsize); // ジャストサイズに調整
        fread(buffer, 1, fsize, fp);
        total_len = fsize;
        fclose(fp);
    }

    *out_len = total_len;
    return buffer;
}

void load_data_to_mpz(const char* filepath, mpz_t out_m) {
    size_t len;
    unsigned char* buf = load_input_data(filepath, &len);
    if (!buf) { fprintf(stderr, "データファイルの読み込み失敗\n"); exit(1); }
    mpz_import(out_m, len, 1, 1, 1, 0, buf);
    free(buf);
}

void save_mpz_to_output(const char* filepath, mpz_t in_c) {
    FILE* fp = stdout;
    if (strcmp(filepath, "-") != 0) {
        fp = fopen(filepath, "wb");
        if (!fp) { perror("出力ファイルを開けません"); exit(1); }
    } else {
        #ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
        #endif
    }

    size_t count;
    unsigned char* buf = mpz_export(NULL, &count, 1, 1, 1, 0, in_c);
    
    // 200万ビット環境（固定長/最大長パディング）を考慮し、
    // 必要に応じて先頭のゼロ埋め（パディングサイズ合わせ）をここで行うとより堅牢になります。
    
    fwrite(buf, 1, count, fp);
    free(buf);

    if (fp != stdout) {
        fclose(fp);
    } else {
        fflush(stdout); // パイプラインが詰まらないよう確実にフラッシュ
    }
}

void save_mpz_to_file(const char* filepath, mpz_t in_c) {
    FILE* fp = fopen(filepath, "wb");
    if (!fp) { perror("出力ファイルを開けません"); exit(1); }
    size_t count;
    unsigned char* buf = mpz_export(NULL, &count, 1, 1, 1, 0, in_c);
    fwrite(buf, 1, count, fp);
    free(buf);
    fclose(fp);
}

// OpenMPを用いたマルチスレッドCRT冪乗余
void crt_powm_parallel(mpz_t out_m, const mpz_t c, const mpz_t dp, const mpz_t dq, const mpz_t p, const mpz_t q, const mpz_t qinv) {
    mpz_t m1, m2, h;
    mpz_inits(m1, m2, h, NULL);

    #pragma omp parallel sections
    {
        #pragma omp section
        { mpz_powm(m1, c, dp, p); }
        #pragma omp section
        { mpz_powm(m2, c, dq, q); }
    }

    mpz_sub(h, m1, m2);
    if (mpz_sgn(h) < 0) mpz_add(h, h, p);
    mpz_mul(h, h, qinv);
    mpz_mod(h, h, p);
    mpz_mul(out_m, h, q);
    mpz_add(out_m, out_m, m2);

    mpz_clears(m1, m2, h, NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        printf("【2097152bit RSA 統合並列CLIツール v3】\n");
        printf("使用法:\n");
        printf("  公開鍵演算(暗号化/復号): %s pub-pow <key.der> <input_file> <output_file>\n", argv[0]);
        printf("  秘密鍵演算(復　号/署名): %s pri-pow <key.der> <input_file> <output_file>\n", argv[0]);
        printf("  ※ ファイルパスに '-' を指定すると標準入出力（パイプ）になります。\n");
        return 1;
    }

    char* mode = argv[1];
    char* key_path = argv[2];
    char* in_path = argv[3];
    char* out_path = argv[4];

    mpz_t m, c, n, e, d, p, q, dp, dq, qinv;
    mpz_inits(m, c, n, e, d, p, q, dp, dq, qinv, NULL);

    size_t der_len;
    unsigned char* der_bytes = load_der_file(key_path, &der_len);
    if (!der_bytes) return 1;

    int max_el = 9;
    mpz_t* raw_elements = malloc(max_el * sizeof(mpz_t));
    for(int i=0; i<max_el; i++) mpz_init(raw_elements[i]);
    
    // 鍵のパース
    int found = parse_pkcs1_key(der_bytes, der_len, raw_elements, max_el);
    free(der_bytes);

    int is_private_key = 0;

    if (found >= 9) {
        // 要素が9個以上ある場合は「秘密鍵」としてマッピング
        is_private_key = 1;
        mpz_set(n, raw_elements[1]); mpz_set(e, raw_elements[2]); mpz_set(d, raw_elements[3]);
        mpz_set(p, raw_elements[4]); mpz_set(q, raw_elements[5]);
        mpz_set(dp, raw_elements[6]); mpz_set(dq, raw_elements[7]); mpz_set(qinv, raw_elements[8]);
        printf("→ 秘密鍵を検出しました (Modulus: %lu bits)\n", (unsigned long)mpz_sizeinbase(n, 2));
    } else if (found >= 2) {
        // 要素が2個（または3個）の場合は「公開鍵」としてマッピング (PKCS#1 RSAPublicKey: n, e)
        is_private_key = 0;
        mpz_set(n, raw_elements[0]);
        mpz_set(e, raw_elements[1]);
        printf("→ 公開鍵を検出しました (Modulus: %lu bits)\n", (unsigned long)mpz_sizeinbase(n, 2));
    } else {
        fprintf(stderr, "エラー: 有効なRSA鍵パラメータが検出されませんでした。(検出要素数: %d)\n", found);
        return 1;
    }

    for(int i=0; i<max_el; i++) mpz_clear(raw_elements[i]); free(raw_elements);

    // 入力データの読み込み
    load_data_to_mpz(in_path, m);

    int ret = 0;
    // モード別の分岐処理（2つに集約）
    if (strcmp(mode, "pub-pow") == 0) {
        printf("公開鍵による冪乗余計算 (m^e mod n) を実行中...\n");
        // 入力（メッセージ、または署名データ）を読み込み
        load_data_to_mpz(in_path, m);
        
        // 計算実行
        mpz_powm(c, m, e, n);
        
        // 結果（暗号文、または復元パディング）を出力
        save_mpz_to_output(out_path, c);
        printf("→ 計算完了\n");

    } else if (strcmp(mode, "pri-pow") == 0) {
        if (!is_private_key) {
            fprintf(stderr, "エラー: pri-pow（秘密鍵演算）には秘密鍵（DER）が必要です。\n");
            return 1;
        }
        printf("[OpenMP] 秘密鍵による並列CRT冪乗余計算 (m^d mod n) を実行中...\n");
        load_data_to_mpz(in_path, m);
        
        // 2コア並列CRT計算
        crt_powm_parallel(c, m, dp, dq, p, q, qinv);

        // Bellcoreフォルト攻撃対策（すべての秘密鍵演算で実行されるため安全！）
        mpz_t test_h; mpz_init(test_h);
        mpz_powm(test_h, c, e, n);
        if (mpz_cmp(test_h, m) != 0) {
            fprintf(stderr, "\n🔥 [CRITICAL] 計算フォルトを検出！出力をブロックします。\n");
            mpz_clear(test_h); return 99;
        }
        mpz_clear(test_h);
        
        save_mpz_to_output(out_path, c);
        printf("→ 計算完了\n");

    } else {
        fprintf(stderr, "未知のモード: %s (使用可能: pub-pow / pri-pow)\n", mode);
        return 1;
    }

    mpz_clears(m, c, n, e, d, p, q, dp, dq, qinv, NULL);
    return ret;
}

