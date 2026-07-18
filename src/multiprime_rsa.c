#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <gmp.h>

#define TARGET_BITS 4194304
#define MIN_PRIME 1000000000000000000ULL       // 10^18
#define MAX_PRIME 9999999999999999999ULL       // 10^19 - 1
#define ESTIMATED_PRIMES 68000                 // 必要素数個数の概算

// トーナメント（二分木）方式で配列内のすべてのGMP整数を高速に乗算する
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
    // 乱数の初期化
    gmp_randstate_t r_state;
    gmp_randinit_default(r_state);
    gmp_randseed_ui(r_state, time(NULL));
    srand(time(NULL));

    printf("[1] 19桁の素数プールを準備中...\n");

    // 19桁の素数を格納する動的配列
    size_t capacity = ESTIMATED_PRIMES + 2000;
    uint64_t *prime_pool = malloc(capacity * sizeof(uint64_t));
    double *log_pool = malloc(capacity * sizeof(double));
    
    mpz_t gmp_p;
    mpz_init(gmp_p);

    size_t prime_count = 0;
    double current_log_sum = 0.0;
    double target_log = (double)TARGET_BITS - 0.5; // ビット境界の中央を狙う

    // 前半：目標の一歩手前まで19桁の素数を敷き詰める
    while (current_log_sum < target_log - 1000.0) {
        uint64_t rand_val = MIN_PRIME + ((uint64_t)rand() << 32 | rand()) % (MAX_PRIME - MIN_PRIME);
        mpz_set_ui(gmp_p, rand_val);
        mpz_nextprime(gmp_p, gmp_p); // 次の素数を一瞬で探索
        
        uint64_t p_val = mpz_get_ui(gmp_p);
        if (p_val <= MAX_PRIME) {
            prime_pool[prime_count] = p_val;
            log_pool[prime_count] = log2((double)p_val);
            current_log_sum += log_pool[prime_count];
            prime_count++;
        }
    }

    printf("  -> 前半戦終了: %zu 個の素数を確保。現在の暫定ビット和: %.2f\n", prime_count, current_log_sum);
    printf("[2] ナップサック（最適化）アルゴリズムによるビタ止め調整中...\n");

    mpz_t N_check;
    mpz_init(N_check);
    
    // 前半の素数を一度結合して現在の正確な N_check を得る
    mpz_t *temp_primes = malloc(prime_count * sizeof(mpz_t));
    for (size_t i = 0; i < prime_count; i++) mpz_init_set_ui(temp_primes[i], prime_pool[i]);
    tournament_multiply(temp_primes, 0, prime_count - 1);
    mpz_set(N_check, temp_primes);
    for (size_t i = 1; i < prime_count; i++) mpz_clear(temp_primes[i]);
    free(temp_primes);

    // 1. ラスト1個（残り61〜62ビット）の手前まで、安全に素数を足していく
    while (1) {
        size_t current_bits = mpz_sizeinbase(N_check, 2);
        
        // 残りが「ラスト1個」で相殺できるサイズ（61〜62ビット）になったらループを抜ける
        if (TARGET_BITS - current_bits >= 61 && TARGET_BITS - current_bits <= 62) {
            break;
        }

        // まだ遠い場合は普通に19桁のランダム素数を追加
        uint64_t rand_val = MIN_PRIME + ((uint64_t)rand() << 32 | rand()) % (MAX_PRIME - MIN_PRIME);
        mpz_set_ui(gmp_p, rand_val);
        mpz_nextprime(gmp_p, gmp_p);
        uint64_t p_val = mpz_get_ui(gmp_p);
        if (p_val > MAX_PRIME) continue;

        mpz_mul_ui(N_check, N_check, p_val);
        
        // もし行き過ぎてラスト1個の調整可能圏内を飛び越えそうになったら、一旦戻してブレイク
        if (mpz_sizeinbase(N_check, 2) > TARGET_BITS - 61) {
            mpz_divexact_ui(N_check, N_check, p_val);
            break;
        }
        prime_pool[prime_count++] = p_val;
    }

    // 2. 【核心】ジャスト4194304ビットにするための最後の素数の「厳密な数値範囲」を割り出す
    mpz_t limit_min, limit_max, target_range;
    mpz_inits(limit_min, limit_max, target_range, NULL);

    // limit_min = 2^4194303 / N_check
    mpz_set_ui(temp, 1);
    mpz_mul_2exp(limit_min, temp, TARGET_BITS - 1); // 2^4194303
    mpz_cdiv_q(limit_min, limit_min, N_check);     // 切り上げ除算

    // limit_max = (2^4194304 - 1) / N_check
    mpz_mul_2exp(limit_max, temp, TARGET_BITS);     // 2^4194304
    mpz_sub_ui(limit_max, limit_max, 1);           // 2^4194304 - 1
    mpz_fdiv_q(limit_max, limit_max, N_check);     // 切り捨て除算

    // 19桁（MIN_PRIME 〜 MAX_PRIME）の制約と交差（交わり）を取る
    mpz_t r_min, r_max;
    mpz_init_set_ui(r_min, MIN_PRIME);
    mpz_init_set_ui(r_max, MAX_PRIME);
    
    if (mpz_cmp(limit_min, r_min) > 0) mpz_set(r_min, limit_min);
    if (mpz_cmp(limit_max, r_max) < 0) mpz_set(r_max, limit_max);

    // 3. 割り出した「合格確定の素数範囲」から、ランダムに最後の素数（ラストピース）を引き当てる
    mpz_sub(target_range, r_max, r_min);
    unsigned long seed_offset = 0;

    while (1) {
        // 合格範囲のサイズを元に、ランダムなオフセットを加える（無限ループ対策の乱数化）
        mpz_urandomm(gmp_p, r_state, target_range);
        mpz_add(gmp_p, gmp_p, r_min);
        
        mpz_nextprime(gmp_p, gmp_p); // 範囲内から素数を抽出

        // 抽出した素数が本当に上限を超えていないか、また19桁の範囲内かチェック
        if (mpz_cmp(gmp_p, r_max) <= 0) {
            uint64_t final_p = mpz_get_ui(gmp_p);
            
            // 念のための最終安全確認
            mpz_mul_ui(N_check, N_check, final_p);
            if (mpz_sizeinbase(N_check, 2) == TARGET_BITS) {
                prime_pool[prime_count++] = final_p;
                break; // 🎉 完全勝利！
            }
            mpz_divexact_ui(N_check, N_check, final_p); // 万が一ズレたら再ガチャ
        }
        
        // 乱数シードが万が一偏ったときのために、明示的にステートを揺らす
        gmp_randseed_ui(r_state, time(NULL) + (++seed_offset));
    }

    mpz_clears(N_check, limit_min, limit_max, target_range, r_min, r_max, NULL);
    printf(" ➔ ビタ止め完了！正確にターゲットへ着弾しました。\n");
    printf("[3] トーナメント方式による超高速結合を実行中...\n");

    // GMP配列の初期化と素数の代入
    mpz_t *gmp_primes = malloc(prime_count * sizeof(mpz_t));
    for (size_t i = 0; i < prime_count; i++) {
        mpz_init_set_ui(gmp_primes[i], prime_pool[i]);
    }

    // 二分木掛け算の開始
    clock_t start_time = clock();
    tournament_multiply(gmp_primes, 0, prime_count - 1);
    clock_t end_time = clock();

    // 最終公開鍵 N のビット長チェック
    size_t final_bits = mpz_sizeinbase(gmp_primes[0], 2);
    printf("\n==================================================\n");
    printf(" 結 果 報 告\n");
    printf("==================================================\n");
    printf(" 使用した19桁の素数の総数 (u): %zu 個\n", prime_count);
    printf(" 合成数 N の実際のビット長   : %zu ビット\n", final_bits);
    printf(" 合成（掛け算）にかかった時間 : %.4f 秒\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    
    if (final_bits == TARGET_BITS) {
        printf(" 判定: 🎉 🎉 🎉 【4194304ビット ジャストビタ止め成功！】 🎉 🎉 🎉\n");
    } else {
        printf(" 判定: ⚠️ わずかにズレました（再実行するかシードを変更してください）\n");
    }
    printf("==================================================\n");

    // 後片付け
    for (size_t i = 0; i < prime_count; i++) {
        // トーナメント乗算の結果、gmp_primes[0] に全て集約されているため[0]以外をクリア
        if (i != 0) mpz_clear(gmp_primes[i]);
    }
    mpz_clear(gmp_primes[0]);
    mpz_clear(gmp_p);
    gmp_randclear(r_state);
    free(prime_pool);
    free(log_pool);
    free(gmp_primes);

    return 0;
}

