#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <gmp.h>

#define TARGET_BITS 4194304
#define MIN_PRIME 1000000000000000000ULL
#define MAX_PRIME 9999999999999999999ULL
#define ESTIMATED_PRIMES 68000

// トーナメント方式高速乗算
void tournament_multiply(mpz_t *arr, size_t start, size_t end) {
    if (start >= end) return;
    if (start + 1 == end) {
        mpz_mul(arr[start], arr[start], arr[end]);
        return;
    }
    size_t mid = start + (end - start) / 2;
    tournament_multiply(arr, start, mid);
    tournament_multiply(arr, mid + 1, end);
    mpz_mul(arr[start], arr[start], arr[mid + 1]);
}

int main() {
    gmp_randstate_t r_state;
    gmp_randinit_default(r_state);
    gmp_randseed_ui(r_state, time(NULL));
    srand(time(NULL));

    printf("🤖 [1/5] 4,194,304ビット(ビタ止め)多素数鍵を生成中...\n");

    size_t capacity = ESTIMATED_PRIMES + 2000;
    uint64_t *prime_pool = malloc(capacity * sizeof(uint64_t));
    
    mpz_t gmp_p;
    mpz_init(gmp_p);
    size_t u = 0;
    double current_log_sum = 0.0;
    double target_log = (double)TARGET_BITS - 0.5;

    // ビタ止め素数収集
    while (1) {
        uint64_t rand_val = MIN_PRIME + ((uint64_t)rand() << 32 | rand()) % (MAX_PRIME - MIN_PRIME);
        mpz_set_ui(gmp_p, rand_val);
        mpz_nextprime(gmp_p, gmp_p);
        uint64_t p_val = mpz_get_ui(gmp_p);
        if (p_val > MAX_PRIME) continue;
        double p_log = log2((double)p_val);

        if (current_log_sum + p_log < (double)TARGET_BITS) {
            prime_pool[u] = p_val;
            current_log_sum += p_log;
            u++;
        } else {
            prime_pool[u] = p_val;
            u++;
            break;
        }
    }

    // 公開鍵 N の作成
    mpz_t *gmp_primes = malloc(u * sizeof(mpz_t));
    for (size_t i = 0; i < u; i++) mpz_init_set_ui(gmp_primes[i], prime_pool[i]);
    
    mpz_t N;
    mpz_init(N);
    // トーナメント乗算のために配列をコピーして結合
    mpz_t *calc_primes = malloc(u * sizeof(mpz_t));
    for (size_t i = 0; i < u; i++) mpz_init_set(calc_primes[i], gmp_primes[i]);
    tournament_multiply(calc_primes, 0, u - 1);
    mpz_set(N, calc_primes[0]);
    
    size_t final_bits = mpz_sizeinbase(N, 2);
    printf(" ➔ 素数個数 (u): %zu 個 / Nのサイズ: %zu ビット\n", u, final_bits);

    if (final_bits != TARGET_BITS) {
        printf("⚠️ ビタ止めに失敗しました。再実行してください。\n");
        return -1;
    }

    // 秘密鍵 d の計算 (λ(N)の計算)
    printf("🤖 [2/5] 秘密鍵指数 d および大量のCRTパラメータを計算中...\n");
    mpz_t lambda, gcd, temp;
    mpz_inits(lambda, gcd, temp, NULL);
    mpz_sub_ui(lambda, gmp_primes[0], 1);
    for(size_t i = 1; i < u; i++) {
        mpz_sub_ui(temp, gmp_primes[i], 1);
        mpz_gcd(gcd, lambda, temp);
        mpz_divexact(lambda, lambda, gcd);
        mpz_mul(lambda, lambda, temp); // lcm
    }
    mpz_t d, e;
    mpz_init_set_ui(e, 65537);
    mpz_init(d);
    mpz_invert(d, e, lambda);

    // 💾 【シリアライズ】自作ファイルへ書き出し
    printf("💾 [3/5] オリジナル規格（.hmpub / .hmpriv）でファイルに保存中...\n");
    
    // 公開鍵保存
    FILE *f_pub = fopen("key.hmpub", "wb");
    uint64_t e_out = 65537;
    uint64_t u_out = u;
    fwrite(&e_out, sizeof(uint64_t), 1, f_pub);
    fwrite(&u_out, sizeof(uint64_t), 1, f_pub);
    mpz_out_raw(f_pub, N);
    fclose(f_pub);

    // 秘密鍵保存
    FILE *f_priv = fopen("key.hmpriv", "wb");
    fwrite(&u_out, sizeof(uint64_t), 1, f_priv);
    mpz_out_raw(f_priv, d);

    // 大量のCRTパラメータを計算しながら、ストリーミングでファイルに直撃書き込み
    mpz_t R, t, d_i, dP, dQ, qInv;
    mpz_inits(R, t, d_i, dP, dQ, qInv, NULL);
    
    // p1, p2 用 (通常のRSAパラメータ)
    mpz_sub_ui(temp, gmp_primes[0], 1); mpz_mod(dP, d, temp);
    mpz_sub_ui(temp, gmp_primes[1], 1); mpz_mod(dQ, d, temp);
    mpz_invert(qInv, gmp_primes[1], gmp_primes[0]);
    
    mpz_out_raw(f_priv, gmp_primes[0]); mpz_out_raw(f_priv, gmp_primes[1]);
    mpz_out_raw(f_priv, dP); mpz_out_raw(f_priv, dQ); mpz_out_raw(f_priv, qInv);

    mpz_mul(R, gmp_primes[0], gmp_primes[1]); // R_3 = p1 * p2
    
    // p3 以降の数万個のループ
    for (size_t i = 2; i < u; i++) {
        mpz_sub_ui(temp, gmp_primes[i], 1);
        mpz_mod(d_i, d, temp);          // d_i = d mod (p_i - 1)
        mpz_invert(t, R, gmp_primes[i]); // t_i = R_i^(-1) mod p_i
        
        mpz_out_raw(f_priv, gmp_primes[i]);
        mpz_out_raw(f_priv, d_i);
        mpz_out_raw(f_priv, t);

        mpz_mul(R, R, gmp_primes[i]); // Rを太らせる
    }
    fclose(f_priv);
    printf(" ➔ 鍵ファイル生成完了! (OpenSSL破砕用秘密鍵 key.hmpriv が爆誕しました)\n");

    // 🧪 【デシリアライズ＆検証】ここからテスト暗号化・復号
    printf("🧪 [4/5] 作成した鍵ファイルを読み直して、400万ビットの暗号化テストを開始...\n");
    
    // 公開鍵の読み込みと擬似メッセージの暗号化
    FILE *f_pub_in = fopen("key.hmpub", "rb");
    fread(&e_out, sizeof(uint64_t), 1, f_pub_in);
    fread(&u_out, sizeof(uint64_t), 1, f_pub_in);
    mpz_t N_in; mpz_init(N_in);
    mpz_inp_raw(N_in, f_pub_in);
    fclose(f_pub_in);

    // テスト平文M (Nより1ビット小さい、巨大なメッセージ)
    mpz_t M, C;
    mpz_inits(M, C, NULL);
    mpz_urandomb(M, r_state, TARGET_BITS - 1);
    
    // 暗号化: C = M^e mod N
    clock_t enc_start = clock();
    mpz_powm(C, M, e, N_in);
    clock_t enc_end = clock();
    printf(" ➔ 暗号化完了 (通常の剰余冪乗): %.4f 秒\n", (double)(enc_end - enc_start) / CLOCKS_PER_SEC);

    // 🔓 秘密鍵を使った「超々多素数Garner復号」
    printf("🔓 [5/5] key.hmpriv(秘密鍵)をストリームロードし、6万段CRT復号ループを決行...\n");
    
    FILE *f_priv_in = fopen("key.hmpriv", "rb");
    fread(&u_out, sizeof(uint64_t), 1, f_priv_in);
    mpz_t d_in; mpz_init(d_in);
    mpz_inp_raw(d_in, f_priv_in);

    clock_t dec_start = clock();

    // p1, p2部分のロードと部分復号
    mpz_t p1, p2, dP_in, dQ_in, qInv_in, m1, m2, h, m_accum;
    mpz_inits(p1, p2, dP_in, dQ_in, qInv_in, m1, m2, h, m_accum, NULL);
    mpz_inp_raw(p1, f_priv_in); mpz_inp_raw(p2, f_priv_in);
    mpz_inp_raw(dP_in, f_priv_in); mpz_inp_raw(dQ_in, f_priv_in); mpz_inp_raw(qInv_in, f_priv_in);

    mpz_powm(m1, C, dP_in, p1);
    mpz_powm(m2, C, dQ_in, p2);
    
    // h = qInv * (m1 - m2) mod p1
    mpz_sub(h, m1, m2);
    mpz_mod(h, h, p1);
    mpz_mul(h, h, qInv_in);
    mpz_mod(h, h, p1);
    // m_accum = m2 + h * p2
    mpz_mul(m_accum, h, p2);
    mpz_add(m_accum, m_accum, m2);

    mpz_t R_dec; mpz_init_set(R_dec, p1); mpz_mul(R_dec, R_dec, p2);

    // p3 〜 p_u までの残りの超大量パラメータを「ファイルから1組ずつ読みながら」CRT合成
    mpz_t p_i, di_in, ti_in, mi, h_i;
    mpz_inits(p_i, di_in, ti_in, mi, h_i, NULL);

    for (size_t i = 2; i < u_out; i++) {
        mpz_inp_raw(p_i, f_priv_in);
        mpz_inp_raw(di_in, f_priv_in);
        mpz_inp_raw(ti_in, f_priv_in);

        // mi = C^di mod p_i
        mpz_powm(mi, C, di_in, p_i);

        // h_i = t_i * (m_i - m_accum) mod p_i
        mpz_sub(h_i, mi, m_accum);
        mpz_mod(h_i, h_i, p_i);
        mpz_mul(h_i, h_i, ti_in);
        mpz_mod(h_i, h_i, p_i);

        // m_accum = m_accum + h_i * R_dec
        mpz_mul(h_i, h_i, R_dec);
        mpz_add(m_accum, m_accum, h_i);

        // R_dec = R_dec * p_i
        mpz_mul(R_dec, R_dec, p_i);
    }
    fclose(f_priv_in);
    clock_t dec_end = clock();
    printf(" ➔ 復号ループ完了 (6万段の積み木マージ): %.4f 秒\n", (double)(dec_end - dec_start) / CLOCKS_PER_SEC);

    // 最終検証: 元の平文Mと、積み上げたm_accumが一致するか？
    if (mpz_cmp(M, m_accum) == 0) {
        printf("\n🎉 🎉 🎉 【完璧!! 400万ビットメッセージの完全復元に成功しました！】 🎉 🎉 🎉\n");
    } else {
        printf("\n❌ 復号データが破損しています。ロジックを確認してください。\n");
    }

    // メモリ解放（一部省略）
    mpz_clears(N, lambda, gcd, temp, d, e, M, C, m_accum, R_dec, NULL);
    free(prime_pool);
    return 0;
}

