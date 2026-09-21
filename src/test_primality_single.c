#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

#include "large_sieve_io.h"

/**
 * mpz_probab_prime_p の反復回数の既定値。
 * GMPは「試し割り → BPSW → (reps-24)回の追加Miller-Rabin」を行うため、
 * 24はBPSWのみで追加ラウンド無しを意味する。素数探索本体
 * (PrimeSearchTask2)と同じ値にして、判定基準を揃えている。
 */
#define DEFAULT_REPS 24

/**
 * @brief 単一数について素数判定
 *
 * 入力は16進数テキスト。10進数のファイルを渡しても構文エラーにならず
 * 別の数として読めてしまうため、load_hex_numberがa-fの有無で警告を出す。
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "使い方: %s <16進数ファイル> [reps]\n", argv[0]);
        fprintf(stderr, "  reps: mpz_probab_prime_pの反復回数 (既定 %d)。\n",
                DEFAULT_REPS);
        fprintf(stderr, "        24でBPSWのみ。25以上は1増えるごとに\n");
        fprintf(stderr, "        Miller-Rabinが1ラウンド増える。\n");
        return EXIT_FAILURE;
    }

    int reps = DEFAULT_REPS;
    if (argc >= 3)
    {
        errno = 0;
        char *end = NULL;
        long v = strtol(argv[2], &end, 10);
        if (errno != 0 || end == argv[2] || *end != '\0' || v < 1 || v > INT_MAX)
        {
            fprintf(stderr, "repsが不正です: %s\n", argv[2]);
            return EXIT_FAILURE;
        }
        reps = (int)v;
    }

    mpz_t primeNumberCandidate;
    mpz_init(primeNumberCandidate);
    if (load_hex_number(argv[1], primeNumberCandidate) != 0)
    {
        mpz_clear(primeNumberCandidate);
        return EXIT_FAILURE;
    }

    int answer = mpz_probab_prime_p(primeNumberCandidate, reps);
    printf("%d\n", answer);
    mpz_clear(primeNumberCandidate);

    return EXIT_SUCCESS;
}
