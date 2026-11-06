
/* 未解決素数ギャップ968を探す旅に出る */
#include <stdio.h>
#include <gmp.h>
#include <time.h>
#include <sys/random.h> // Linuxなどで高品質なシードを取得するため

int main() {
    gmp_randstate_t state;
    mpz_t lower_bound, upper_bound, range, random_num, start_prime, next_prime;

    // 1. 乱数状態の初期化
    gmp_randinit_default(state);

    // 2. 高品質なシードの設定（Linux/POSIX環境の /dev/urandom から取得）
    unsigned long seed;
    if (getrandom(&seed, sizeof(seed), 0) < 0) {
        // 万が一 getrandom が失敗した場合は時刻をバックアップに使用
        seed = (unsigned long)time(NULL);
    }
    gmp_randseed_ui(state, seed);

    // 3. 多倍長整数の初期化
    mpz_init(lower_bound);
    mpz_init(upper_bound);
    mpz_init(range);
    mpz_init(random_num);
    mpz_init(start_prime);
    mpz_init(next_prime);

    // 4. 24桁の範囲を設定 (10^23 から 10^24 - 1)
    mpz_ui_pow_ui(lower_bound, 10, 23); // 10^23 (最小の24桁の数)
    mpz_ui_pow_ui(upper_bound, 10, 24); // 10^24

    // 範囲の幅を計算: range = upper_bound - lower_bound
    mpz_sub(range, upper_bound, lower_bound);

    // 5. 範囲内の乱数を生成 (0 〜 range-1)
    mpz_urandomm(random_num, state, range);

    // 下限を足して、実際の24桁のランダムな数にする
    mpz_add(random_num, random_num, lower_bound);

    // 24桁の生成された乱数を表示
    gmp_printf("Generated 24-digit random base: %Zd\n", random_num);

    // 6. この乱数を起点に、次の素数を探す
    // (mpz_nextprime は Miller-Rabin 判定等を用いて次の素数を高速に見つけます)
    mpz_nextprime(start_prime, random_num);
    gmp_printf("Start Prime for Gap Search    : %Zd\n", start_prime);

    // --- ここからあなたの「ギャップ968」探索ロジックを走らせます ---
    // 例: start_prime から順に次の素数を計算していき、差が968になるかチェックする等

    // 7. メモリ解放
    mpz_clears(lower_bound, upper_bound, range, random_num, start_prime, NULL);
    gmp_randclear(state);

    return 0;
}
