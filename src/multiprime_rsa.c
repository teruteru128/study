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

    // 終盤：ターゲットの対数に極限まで近づけるためのローカルサーチ
    while (1) {
        uint64_t rand_val = MIN_PRIME + ((uint64_t)rand() << 32 | rand()) % (MAX_PRIME - MIN_PRIME);
        mpz_set_ui(gmp_p, rand_val);
        mpz_nextprime(gmp_p, gmp_p);
        uint64_t p_val = mpz_get_ui(gmp_p);
        
        if (p_val > MAX_PRIME) continue;
        double p_log = log2((double)p_val);

        // もしこの素数を入れても目標を超えない、または微調整の範囲なら追加
        if (current_log_sum + p_log < (double)TARGET_BITS) {
            prime_pool[prime_count] = p_val;
            log_pool[prime_count] = p_log;
            current_log_sum += p_log;
            prime_count++;
        } else {
            // オーバーしたら最後のピースを確定し、ループを抜ける
            prime_pool[prime_count] = p_val;
            log_pool[prime_count] = p_log;
            current_log_sum += p_log;
            prime_count++;
            break;
        }
    }

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

