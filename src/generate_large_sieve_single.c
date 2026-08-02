
#include "bitset.h"
#include <time.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "bitsieve.h"
#include "timeutil.h"
#include <endian.h>
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
  uint64_t *primes; // 復元された素数の配列（またはビットマップの断片）
  size_t count; // この塊に含まれる素数の数
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
 * size->bit単位
 */
uint64_t sieve_search(uint64_t *sieve, size_t size, size_t start) {
  if (start >= size) {
    return (size_t)-1;
  }
  size_t index = start;
  do {
    if ((sieve[unitIndex(index)] & bit(index)) != 0)
      return index;
    index++;
  } while (index < size);
  return (size_t)-1;
}

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

  char *smallSievePath = argv[3];
  FILE *smallSieveFile = fopen(smallSievePath, "rb");
  if (smallSieveFile == NULL) {
    perror("small");
    return 1;
  }
  uint64_t bitcount_in = 0;
  size_t readcount = fread(&bitcount_in, sizeof(uint64_t), 1, smallSieveFile);
  if (readcount < 0) {
    perror("small sieve haeder");
    return 1;
  }

  const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
  fprintf(stderr, "large sieve size: %zu bits\n", searchLength);
  // printf("%lu\n", searchLength);
  // struct BitSieve *largeSieve = bs_new();
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
  // bs_initInstance(largeSieve, base, searchLength);

  size_t start = 0;
  uint64_t primebuffer[128];
  readcount = fread(primebuffer, sizeof(uint64_t), 128, smallSieveFile);
  // エンディアン変換
  for (size_t i = 0; i < readcount; i++) {
    primebuffer[i] = be64toh(primebuffer[i]);
  }
  // 素数取得
  // 断片オフセット
  size_t fragment_offset = 0;
  size_t step = 0;
  uint64_t prime = 0;

  // sizeが要素単位なのかバイト単位なのかビット単位なのかわからんねん！！
  step = sieve_search(primebuffer, readcount * 64, start);
  // fragment_offsetが要素単位なら*64だしバイト単位なら*8だしビット単位なら*1なんだが
  prime = (fragment_offset + step) * 2ULL + 1ULL;

  do {
    start = prime - mpz_fdiv_ui(base, prime);
    if ((start & 1UL) == 0UL)
      start += prime;
    if ((start - 1) / 2 < searchLength)
      fprintf(stderr, "start is over in search length\n");
    // sieve single
    while(start < readcount * 64) {
        worker_outputs[0][unitIndex(start)] |= bit(start);
        start += prime;
    }
    // sieve search
    // calc next prime
  } while (step != (size_t)-1);
  
  fragment_offset += readcount;

  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);
  struct timespec diff;
  difftimespec(&diff, &finish, &startt);
  clock_gettime(CLOCK_REALTIME, &finish);
  localtime_r(&finish.tv_sec, &tm);
  strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
  fprintf(stderr, "%s: 篩の初期化を完了しました. (%ld.%09lds)\n", time_buffer,
          diff.tv_sec, diff.tv_nsec);
  fclose(smallSieveFile);

#if 0
  char *outfilename = argv[2];
  {
    FILE *fout = fopen(outfilename, "wb");
    //bs_fileout(fout, largeSieve);
    fclose(fout);
  }
  // bs_free(largeSieve);
#endif
  mpz_clear(base);
  return 0;
}
