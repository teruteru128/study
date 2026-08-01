
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <sys/random.h>
#include <time.h>

#define TARGET_BITS 4194304
#define ARRAY_NUM 100000                       // 素数プールのサイズ
#define MIN_PRIME 1000000000000000000ULL       // 10^18
#define MAX_PRIME 9999999999999999999ULL       // 10^19 - 1

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

int main(int argc, char *argv[]) {
    mpz_t p, pmin, pmax, window;
    gmp_randstate_t state;
    mpz_inits(p, pmin, pmax, window, NULL);
    gmp_randinit_default(state);
    unsigned char buffer[2493];
    ssize_t len = getrandom(buffer, 2493, GRND_NONBLOCK);
    if (len < 0) {
        perror("getrandom fail");
        return 1;
    }
    mpz_import(p, 2493, 1, 1, 0, 0, buffer);
    gmp_randseed(state, p);
    mpz_set_ui(p, 0);
    mpz_ui_pow_ui(pmin, 10, 18);
    mpz_ui_pow_ui(pmax, 10, 19);
    mpz_sub(window, pmax, pmin);

    uint64_t *prime_pool = malloc(ARRAY_NUM * sizeof(uint64_t));
    double *log_pool = malloc(ARRAY_NUM * sizeof(double));
    size_t prime_count = 0;
    double sum_of_log = 0.;
    uint64_t prime = 0;
    // (2^22)-1*0.99
    double firsttarget = 4152359.96;
    while(sum_of_log < firsttarget){
        mpz_urandomm(p, state, window);
        mpz_add(p, p, pmin);
        do {
            mpz_nextprime(p, p);
        } while(mpz_fdiv_ui(p, 65537) == 1);
        prime = mpz_get_ui(p);
        if(prime <= MAX_PRIME){
            prime_pool[prime_count] = prime;
            log_pool[prime_count] = log2((double)prime);
            sum_of_log += log_pool[prime_count];
            prime_count++;
        }
    }
    
    fprintf(stderr, "暫定 %zu 個の素数を確保。現在の暫定ビット和: %.2f\n", prime_count, sum_of_log);
    fprintf(stderr, "[2] ナップサック（最適化）アルゴリズムによるビタ止め調整中...\n");

    while(1) {
        mpz_urandomm(p, state, window);
        mpz_add(p, p, pmin);
        do {
            mpz_nextprime(p, p);
        } while(mpz_fdiv_ui(p, 65537) == 1);
        prime = mpz_get_ui(p);
        if(prime > MAX_PRIME) continue;

        double p_log = log2((double) prime);

        double tmp_sum_of_log = sum_of_log + p_log;
        if(tmp_sum_of_log < (double)(TARGET_BITS - 1)) {
            // 目標を超えない
            prime_pool[prime_count] = prime;
            log_pool[prime_count] = p_log;
            sum_of_log = tmp_sum_of_log;
            prime_count++;
        } else if((double)(TARGET_BITS - 1) <= tmp_sum_of_log && tmp_sum_of_log < (double)TARGET_BITS){
            // 目標に入った
            prime_pool[prime_count] = prime;
            log_pool[prime_count] = p_log;
            sum_of_log = tmp_sum_of_log;
            prime_count++;
            break;
        } else {
            // 目標をオーバーした
			fprintf(stderr, "over: %.2f, %.2f, %.2f, 10個戻ります\n", sum_of_log, p_log, tmp_sum_of_log);
            prime_count -= 10;
            sum_of_log = 0;
            for(size_t i = 0; i < prime_count; i++) {
                sum_of_log += log_pool[i];
            }
            continue;
        }
    }

   fprintf(stderr, "[3] トーナメント方式による超高速結合を実行中...\n");
	mpz_t *gmp_primes = malloc(prime_count * sizeof(mpz_t));
    for(size_t i = 0; i < prime_count; i++) {
        mpz_init_set_ui(gmp_primes[i], prime_pool[i]);
    }

    clock_t start_time = clock();
    tournament_multiply(gmp_primes, 0, prime_count - 1);
    clock_t end_time = clock();

    size_t final_bits = mpz_sizeinbase(gmp_primes[0], 2);
    fprintf(stderr, "\n==================================================\n");
    fprintf(stderr, " 結 果 報 告\n");
    fprintf(stderr, "==================================================\n");
    fprintf(stderr, " 使用した19桁の素数の総数 (u): %zu 個\n", prime_count);
    fprintf(stderr, " 合成数 N の実際のビット長   : %zu ビット\n", final_bits);
    fprintf(stderr, " 合成（掛け算）にかかった時間 : %.4f 秒\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    
	if (final_bits == TARGET_BITS) {
        fprintf(stderr, " 判定: 🎉 🎉 🎉 【4194304ビット ジャストビタ止め成功！】 🎉 🎉 🎉\n");
    } else {
        fprintf(stderr, " 判定: ⚠️ わずかにズレました（再実行するかシードを変更してください）\n");
    }
    fprintf(stderr, "==================================================\n");
	for(size_t i = 0; i < prime_count; i++) {
		fprintf(stdout, "%" PRIu64 "\n", prime_pool[i]);
    }
	//gmp_fprintf(stderr, "%Zu\n", gmp_primes[0]);
	for(size_t i = 0; i < prime_count; i++) {
		mpz_clear(gmp_primes[i]);
	}
	mpz_clears(p, pmin, pmax, window, NULL);
	gmp_randclear(state);
	free(prime_pool);
    free(log_pool);
    free(gmp_primes);
    return 0;
}
