
#include "bitset.h"
#include "large_sieve_io.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "timeutil.h"
#include <endian.h>
#include <gmp.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// 1スレッドあたりの目安チャンク数。チャンクを細かく刻んでおくことで、
// 素数の大きさによって1件あたりのコストが偏っていても
// (小さい素数ほどlargeSieveへの篩い落としループが長く回る)
// 動的にチャンクを取り合うことで自然に負荷が均される。
#define CHUNKS_PER_THREAD 200

struct WorkerContext {
  const mpz_t *base;      // 読み取り専用で全スレッドから共有
  uint64_t *file_primes;  // 読み取り専用で全スレッドから共有(big-endian格納のまま)
  size_t total_elements;
  size_t chunkSize;
  size_t numChunks;
  atomic_size_t nextChunk; // 次に取得すべきチャンク番号(ロックフリー)
  atomic_size_t chunksDone; // 処理済みチャンク数(進捗表示用)
  size_t searchLength;
  size_t largeSieveElementNum;
};

static void *worker_main(void *arg) {
  struct WorkerContext *ctx = (struct WorkerContext *)arg;

  // スレッドごとに専用のバッファへ書き込む(共有配列への同時書き込みを避ける)
  uint64_t *localSieve = calloc(ctx->largeSieveElementNum, sizeof(uint64_t));
  if (localSieve == NULL) {
    perror("worker local sieve calloc");
    exit(1);
  }

  for (;;) {
    size_t chunkIndex = atomic_fetch_add(&ctx->nextChunk, 1);
    if (chunkIndex >= ctx->numChunks)
      break;

    size_t iStart = chunkIndex * ctx->chunkSize;
    size_t iEnd = iStart + ctx->chunkSize;
    if (iEnd > ctx->total_elements)
      iEnd = ctx->total_elements;

    for (size_t i = iStart; i < iEnd; i++) {
      uint64_t element = be64toh(ctx->file_primes[i]);
      if (element == (uint64_t)-1) {
        // 全ビット合成数
        continue;
      }
      uint64_t mask = 1ULL;
      for (size_t j = 0; j < 64; j++, mask <<= 1) {
        if ((element & mask) != 0) {
          // 合成数
          continue;
        }
        uint64_t prime = (i * 64 + j) * 2 + 1;
        uint64_t start = prime - mpz_fdiv_ui(*ctx->base, prime);
        if ((start & 1ULL) == 0ULL)
          start += prime;

        start = (start - 1) / 2;
        while (start < ctx->searchLength) {
          localSieve[unitIndex(start)] |= bit(start);
          start += prime;
        }
      }
    }
    atomic_fetch_add(&ctx->chunksDone, 1);
  }

  return localSieve;
}

/**
 * @brief large-sieveをマルチスレッドで生成してファイルに書き出し
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr,
            "%s <input even number file> <input small sieve file> <output "
            "large sieve file> [threads]\n",
            argv[0]);
    return EXIT_FAILURE;
  }
  char *base_filename = argv[1];
  char *smallSievePath = argv[2];
  char *outfilename = argv[3];

  size_t numThreads = 0;
  if (argc >= 5) {
    long requested = strtol(argv[4], NULL, 10);
    if (requested > 0)
      numThreads = (size_t)requested;
  }
  if (numThreads == 0) {
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    numThreads = (nproc > 0) ? (size_t)nproc : 1;
  }

  mpz_t base;
  mpz_init(base);
  if (load_even_base(base_filename, base) != 0) {
    mpz_clear(base);
    return 1;
  }
  fprintf(stderr, "even number is %zu bits\n", mpz_sizeinbase(base, 2));
  fprintf(stderr, "threads: %zu\n", numThreads);

  struct SmallSieve smallSieve;
  if (small_sieve_open(smallSievePath, &smallSieve) != 0) {
    mpz_clear(base);
    return 1;
  }

  const size_t searchLength = (size_t)(mpz_sizeinbase(base, 2) / 20.0 * 64);
  size_t largeSieveElementNum = ((searchLength - 1) / 64) + 1;
  fprintf(stderr, "large sieve size: %zu bits\n", searchLength);
  fprintf(stderr, "large sieve elements: %zu elements\n", largeSieveElementNum);
  fprintf(stderr, "small sieve size: %zu bits\n", smallSieve.total_elements * 64);

  struct WorkerContext ctx;
  ctx.base = (const mpz_t *)&base;
  ctx.file_primes = smallSieve.primes;
  ctx.total_elements = smallSieve.total_elements;
  ctx.searchLength = searchLength;
  ctx.largeSieveElementNum = largeSieveElementNum;

  size_t numChunks = numThreads * CHUNKS_PER_THREAD;
  size_t chunkSize = (ctx.total_elements + numChunks - 1) / numChunks;
  if (chunkSize == 0)
    chunkSize = 1;
  ctx.chunkSize = chunkSize;
  ctx.numChunks = (ctx.total_elements + chunkSize - 1) / chunkSize;
  atomic_init(&ctx.nextChunk, (size_t)0);
  atomic_init(&ctx.chunksDone, (size_t)0);

  struct timespec startt;
  {
    struct timespec now;
    struct tm tm;
    char time_buffer[64];
    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &tm);
    strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(stderr,
            "[%s] 基準偶数に対するlarge-sieveの生成を開始します...(チャンク数: "
            "%zu, チャンクサイズ: %zu要素)\n",
            time_buffer, ctx.numChunks, ctx.chunkSize);
  }
  clock_gettime(CLOCK_MONOTONIC, &startt);

  pthread_t *threads = calloc(numThreads, sizeof(pthread_t));
  if (threads == NULL) {
    perror("threads calloc");
    exit(1);
  }
  for (size_t t = 0; t < numThreads; t++) {
    int rc = pthread_create(&threads[t], NULL, worker_main, &ctx);
    if (rc != 0) {
      fprintf(stderr, "pthread_create failed: %d\n", rc);
      exit(1);
    }
  }

  uint64_t *largeSieve = calloc(largeSieveElementNum, sizeof(uint64_t));
  if (largeSieve == NULL) {
    perror("large sieve calloc error");
    exit(1);
  }

  // 簡易進捗表示: 処理済みチャンク数を一定間隔でポーリングして出力する
  const int PROGRESS_INTERVAL_SEC = 10;
  for (;;) {
    sleep(PROGRESS_INTERVAL_SEC);
    size_t done = atomic_load(&ctx.chunksDone);

    struct timespec nowMono;
    clock_gettime(CLOCK_MONOTONIC, &nowMono);
    struct timespec elapsed;
    difftimespec(&elapsed, &nowMono, &startt);

    struct timespec now;
    struct tm tm;
    char time_buffer[64];
    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &tm);
    strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);

    double percent = 100.0 * (double)done / (double)ctx.numChunks;
    double elapsedSec = (double)elapsed.tv_sec + (double)elapsed.tv_nsec / 1e9;
    if (done > 0) {
      double etaSec = elapsedSec * (double)ctx.numChunks / (double)done - elapsedSec;
      fprintf(stderr,
              "[%s] %zu/%zu chunks done (%.2f %%), elapsed %.1fs, ETA %.1fs\n",
              time_buffer, done, ctx.numChunks, percent, elapsedSec, etaSec);
    } else {
      fprintf(stderr, "[%s] %zu/%zu chunks done (%.2f %%), elapsed %.1fs\n",
              time_buffer, done, ctx.numChunks, percent, elapsedSec);
    }

    if (done >= ctx.numChunks)
      break;
  }

  for (size_t t = 0; t < numThreads; t++) {
    void *retval = NULL;
    pthread_join(threads[t], &retval);
    uint64_t *localSieve = (uint64_t *)retval;
    for (size_t k = 0; k < largeSieveElementNum; k++) {
      largeSieve[k] |= localSieve[k];
    }
    free(localSieve);
  }
  free(threads);

  struct timespec finish;
  clock_gettime(CLOCK_MONOTONIC, &finish);
  struct timespec diff;
  difftimespec(&diff, &finish, &startt);
  {
    struct timespec now;
    struct tm tm;
    char time_buffer[64];
    clock_gettime(CLOCK_REALTIME, &now);
    localtime_r(&now.tv_sec, &tm);
    strftime(time_buffer, 64, "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(stderr, "%s: 篩の初期化を完了しました. (%ld.%09lds)\n", time_buffer,
            diff.tv_sec, diff.tv_nsec);
  }

  size_t bitcnt = 0;
  for (size_t k = 0; k < largeSieveElementNum; k++) {
    bitcnt += __builtin_popcountll(~largeSieve[k]);
  }
  fprintf(stderr, "Remaining prime number candidates: %zu(%f %%)\n", bitcnt,
          (double)bitcnt / searchLength * 100.);

  int writeResult =
      write_large_sieve(outfilename, largeSieve, largeSieveElementNum, searchLength);

  mpz_clear(base);
  free(largeSieve);
  small_sieve_close(&smallSieve);
  return writeResult == 0 ? 0 : 1;
}
