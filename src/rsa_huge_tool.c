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

// PKCS#1 DERから巨大整数を抽出するパーサー
int parse_pkcs1_private_key(const unsigned char* der, size_t der_len, mpz_t* elements, int max_elements) {
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

// データファイルからmpzへインポートする関数
void load_data_to_mpz(const char* filepath, mpz_t out_m) {
    size_t len;
    unsigned char* buf = load_der_file(filepath, &len);
    if (!buf) { fprintf(stderr, "データファイルの読み込み失敗\n"); exit(1); }
    mpz_import(out_m, len, 1, 1, 1, 0, buf);
    free(buf);
}

// mpzのデータをファイルへ書き出す関数
void save_mpz_to_file(const char* filepath, mpz_t in_c) {
    FILE* fp = fopen(filepath, "wb");
    if (!fp) { perror("出力ファイルを開けません"); exit(1); }
    size_t count;
    unsigned char* buf = mpz_export(NULL, &count, 1, 1, 1, 0, in_c);
    fwrite(buf, 1, count, fp);
    free(buf);
    fclose(fp);
}

// OpenMPを用いたマルチスレッドCRT冪乗余（核心部分）
void crt_powm_parallel(mpz_t out_m, const mpz_t c, const mpz_t dp, const mpz_t dq, const mpz_t p, const mpz_t q, const mpz_t qinv) {
    mpz_t m1, m2, h;
    mpz_inits(m1, m2, h, NULL);

    // #pragma omp parallel sections により、2つのコアでp側とq側の100万bit冪乗を完全に同時並列実行
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            mpz_powm(m1, c, dp, p); // コアA担当
        }
        #pragma omp section
        {
            mpz_powm(m2, c, dq, q); // コアB担当
        }
    }

    // CRT合成（ここは一瞬）
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
        printf("【2097152bit RSA 高速並列CLIツール】\n");
        printf("使用法:\n");
        printf("  暗号化: %s enc <key.der> <input_file> <output_file>\n", argv[0]);
        printf("  復  号: %s dec <key.der> <input_file> <output_file>\n", argv[0]);
        printf("  署  名: %s sign <key.der> <input_file> <output_file>\n", argv[0]);
        printf("  検  証: %s verify <key.der> <input_file> <signature_file>\n", argv[0]);
        return 1;
    }

    char* mode = argv[1];
    char* key_path = argv[2];
    char* in_path = argv[3];
    char* out_path = argv[4];

    // GMP変数の初期化
    mpz_t m, c, n, e, d, p, q, dp, dq, qinv;
    mpz_inits(m, c, n, e, d, p, q, dp, dq, qinv, NULL);

    // 鍵のパース
    size_t der_len;
    unsigned char* der_bytes = load_der_file(key_path, &der_len);
    if (!der_bytes) return 1;

    int max_el = 9;
    mpz_t* raw_elements = malloc(max_el * sizeof(mpz_t));
    for(int i=0; i<max_el; i++) mpz_init(raw_elements[i]);
    int found = parse_pkcs1_private_key(der_bytes, der_len, raw_elements, max_el);
    free(der_bytes);

    if (found < 9) {
        fprintf(stderr, "エラー: 秘密鍵の全パラメータ(9個)をパースできませんでした。公開鍵は指定できません。\n");
        return 1;
    }

    mpz_set(n, raw_elements[1]); mpz_set(e, raw_elements[2]); mpz_set(d, raw_elements[3]);
    mpz_set(p, raw_elements[4]); mpz_set(q, raw_elements[5]);
    mpz_set(dp, raw_elements[6]); mpz_set(dq, raw_elements[7]); mpz_set(qinv, raw_elements[8]);

    for(int i=0; i<max_el; i++) mpz_clear(raw_elements[i]); free(raw_elements);

    // 入力データの読み込み
    load_data_to_mpz(in_path, m);

    // 各モードの実行
    if (strcmp(mode, "enc") == 0) {
        printf("[OpenMP] 公開鍵による高速暗号化を実行中...\n");
        mpz_powm(c, m, e, n);
        save_mpz_to_file(out_path, c);
        printf("→ 暗号化完了。ファイルを保存しました: %s\n", out_path);

    } else if (strcmp(mode, "dec") == 0) {
        printf("[OpenMP] 2コア並列CRTによる高速復号を実行中...\n");
        crt_powm_parallel(c, m, dp, dq, p, q, qinv);
        save_mpz_to_file(out_path, c);
        printf("→ 復号完了。ファイルを保存しました: %s\n", out_path);

    } else if (strcmp(mode, "sign") == 0) {
        printf("[OpenMP] 2コア並列CRTによる署名生成を実行中...\n");
        crt_powm_parallel(c, m, dp, dq, p, q, qinv);

        // ⚠️ Bellcoreフォルト攻撃対策：出力前に公開鍵 e でセルフ検証を行う
        mpz_t test_h;
        mpz_init(test_h);
        mpz_powm(test_h, c, e, n); // s^e mod n

        if (mpz_cmp(test_h, m) != 0) {
            fprintf(stderr, "\n🔥 [CRITICAL WARNING] 計算中にビット反転（フォルト）を検出しました！\n");
            fprintf(stderr, "秘密鍵漏洩を防ぐため、壊れた署名の出力をブロックし、処理を強制遮断します。\n");
            mpz_clear(test_h);
            return 99;
        }
        printf("→ セルフ検証クリア（整合性確認完了）。安全な署名です。\n");
        save_mpz_to_file(out_path, c);
        printf("→ 署名完了。ファイルを保存しました: %s\n", out_path);
        mpz_clear(test_h);

    } else if (strcmp(mode, "verify") == 0) {
        printf("[OpenMP] 署名の検証を実行中...\n");
        mpz_t sig;
        mpz_init(sig);
        load_data_to_mpz(out_path, sig); // 第5引数相当（署名ファイル）

        mpz_powm(c, sig, e, n); // s^e mod n

        if (mpz_cmp(c, m) == 0) {
            printf("🟢 【検証成功】 署名は正当であり、データは改ざんされていません。\n");
        } else {
            printf("🔴 【検証失敗】 署名が不正か、データが書き換えられています！\n");
        }
        mpz_clear(sig);
    } else {
        fprintf(stderr, "未知のモードです: %s\n", mode);
    }

    mpz_clears(m, c, n, e, d, p, q, dp, dq, qinv, NULL);
    return 0;
}

