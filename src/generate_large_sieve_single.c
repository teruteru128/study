
#include <time.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bitsieve.h"
#include "timeutil.h"
#include <gmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <stdint.h>

#define MAX_QUEUE_SIZE 4 // キューに貯めておけるタスクの最大数
#define NUM_WORKERS 1    // ワーカースレッドの数

// 1つのタスク（メインからワーカーへ渡す素数リストの塊）
typedef struct {
    uint64_t *primes;   // 復元された素数の配列（またはビットマップの断片）
    size_t count;       // この塊に含まれる素数の数
} SieveTask;

// スレッド間で共有するタスクキュー
SieveTask task_queue[MAX_QUEUE_SIZE];
int queue_head = 0;
int queue_tail = 0;
int queue_count = 0;
int is_shutdown = 0; // ファイル読み込みがすべて終わったら1にする

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;

// 各ワーカースレッドの成果物バッファ（スレッドごとに独立、約820KB）
uint64_t worker_outputs[NUM_WORKERS][104858]; 

/**
 * @brief large-sieveを生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "%s <input even number file> <output large sieve file>\n",
            argv[0]);
    return EXIT_FAILURE;
  }
  mpz_t base;
  mpz_init(base);

  char *base_filename = argv[1];
  {
    FILE *fin = fopen(base_filename, "r");
    if (fin == NULL) {
      perror("load even number");
      mpz_clear(base);
      return 1;
    }
    mpz_inp_str(base, fin, 16);
    fclose(fin);
    fin = NULL;
  }
  if (mpz_cmp_ui(base, 0) == 0) {
    fprintf(stderr, "why? base is zero.\n");
    mpz_clear(base);
    return 1;
  }
  if (mpz_odd_p(base)) {
    fprintf(stderr, "base is odd.\n");
    mpz_clear(base);
    return 1;
  }
  fprintf(stderr, "even number is %zu bits\n", mpz_sizeinbase(base, 2));

  const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
  fprintf(stderr, "large sieve size: %zu bits\n", searchLength);
  // printf("%lu\n", searchLength);
  struct BitSieve *largeSieve = bs_new();
  struct timespec startt;
  struct tm tm;
  clock_gettime(CLOCK_REALTIME, &startt);
  localtime_r(&startt.tv_sec, &tm);
  char time_buffer[64];
  strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(stderr, "%s: 基準偶数に対するlarge-sieveの生成を開始します...\n",
          time_buffer);
  clock_gettime(CLOCK_MONOTONIC, &startt);
  // ファイル読み進めながら篩い分けするの〜？
  bs_initInstance(largeSieve, base, searchLength);

  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);
  struct timespec diff;
  difftimespec(&diff, &finish, &startt);
  clock_gettime(CLOCK_REALTIME, &finish);
  localtime_r(&finish.tv_sec, &tm);
  strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(stderr, "%s: 篩の初期化を完了しました. (%ld.%09lds)\n", time_buffer,
          diff.tv_sec, diff.tv_nsec);

  char *outfilename = argv[2];
  {
    FILE *fout = fopen(outfilename, "wb");
    bs_fileout(fout, largeSieve);
    fclose(fout);
  }
  bs_free(largeSieve);
  mpz_clear(base);
  return 0;
}
