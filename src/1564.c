
#include <gmp.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <stdlib.h>
#include <time.h>

#define ELM 1024
#define BITS (ELM * 64)

int main(int argc, char *argv[], char *envp[])
{
  if (argc < 3)
  {
    return 1;
  }
  char *path = argv[1];
  FILE *in = fopen(path, "rb");
  if (in == NULL)
  {
    perror("fopen");
    return 1;
  }
  mpz_t p, term1;
  mpz_init(p);
  mpz_init(term1);
  mpz_fac_ui(term1, 1564);
  mpz_add_ui(p, term1, strtoull(argv[2], NULL, 10));
  uint64_t primes[ELM];
  if (argc >= 4)
  {
    // 因数除去
    mpz_t fac;
    mpz_init_set_str(fac, argv[3], 10);
    if (mpz_divisible_p(p, fac))
    {
      mpz_remove(p, p, fac);
      fprintf(stderr, "%sがremoveされました\n", argv[3]);
    }
    else
    {
      fprintf(stderr, "%sでremoveできませんでした\n", argv[3]);
    }
    mpz_clear(fac);
  }
  fseek(in, 8, SEEK_SET);
  size_t num = 0;
  struct timespec start, finish, diff;
  clock_gettime(CLOCK_MONOTONIC, &start);
  mpz_t remove_factor;
  mpz_init(remove_factor);
  if (mpz_even_p(p))
  {
    // 偶数除け
    mpz_set_ui(remove_factor, 2);
    uint64_t cnt = mpz_remove(p, p, remove_factor);
    printf("div: %zu(%" PRIu64 ")\n", 2L, cnt);
  }
  // すべての素数をチェックするかpが1になったら終了
  while (num < 137438953280ULL && mpz_cmp_ui(p, 1) > 0)
  {
    size_t len = fread(primes, sizeof(uint64_t), ELM, in);
    for (int i = 0; i < len; i++)
    {
      primes[i] = be64toh(primes[i]);
    }
    size_t bits = len * 64;
    for (int index = 0; index < bits; index++)
    {
      if (((primes[index >> 6] >> (index & 0x3f)) & 1) == 0)
      {
        if (mpz_divisible_ui_p(p, (num + index) * 2 + 1) != 0)
        {
          mpz_set_ui(remove_factor, (num + index) * 2 + 1);
          uint64_t cnt = mpz_remove(p, p, remove_factor);
          clock_gettime(CLOCK_MONOTONIC, &finish);
          if (finish.tv_nsec - start.tv_nsec < 0)
          {
            diff.tv_sec = finish.tv_sec - start.tv_sec - 1;
            diff.tv_nsec = finish.tv_nsec - start.tv_nsec + 1000000000UL;
          }
          else
          {
            diff.tv_sec = finish.tv_sec - start.tv_sec;
            diff.tv_nsec = finish.tv_nsec - start.tv_nsec;
          }
          printf("div: %zu(%" PRIu64 ") %ld.%09ld\n", (num + index) * 2 + 1, cnt, diff.tv_sec, diff.tv_nsec);
        }
      }
    }
    num += bits;
  }
done:
  fclose(in);
  mpz_clear(p);
  mpz_clear(term1);
  mpz_clear(remove_factor);
  return 0;
}
