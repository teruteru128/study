
#include "large_sieve_io.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <endian.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int load_even_base(const char *path, mpz_t base) {
  FILE *fin = fopen(path, "r");
  if (fin == NULL) {
    perror("load even number");
    return -1;
  }
  mpz_inp_str(base, fin, 16);
  fclose(fin);

  if (mpz_cmp_ui(base, 0) == 0) {
    fprintf(stderr, "why? base is zero.\n");
    return -1;
  }
  if (mpz_odd_p(base)) {
    fprintf(stderr, "base is odd.\n");
    return -1;
  }
  return 0;
}

int small_sieve_open(const char *path, struct SmallSieve *sieve) {
  sieve->fd = -1;
  sieve->map_start = NULL;
  sieve->file_size = 0;
  sieve->total_elements = 0;
  sieve->primes = NULL;

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("small sieve open failed");
    return -1;
  }

  struct stat st;
  if (fstat(fd, &st) != 0) {
    perror("fstat small sieve");
    close(fd);
    return -1;
  }
  if (st.st_size < (off_t)sizeof(uint64_t) ||
      ((size_t)st.st_size - sizeof(uint64_t)) % sizeof(uint64_t) != 0) {
    fprintf(stderr, "small sieve file size is invalid: %jd bytes\n",
            (intmax_t)st.st_size);
    close(fd);
    return -1;
  }
  size_t file_size = (size_t)st.st_size;
  size_t total_elements = (file_size - sizeof(uint64_t)) / sizeof(uint64_t);

  void *map_start = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
  if (map_start == MAP_FAILED) {
    perror("small sieve map failed");
    close(fd);
    return -1;
  }

  uint64_t header = be64toh(*(uint64_t *)map_start);
  if (header != (uint64_t)total_elements * 64) {
    fprintf(stderr,
            "small sieve header mismatch: header says %" PRIu64
            " bits, but file size implies %zu bits\n",
            header, total_elements * 64);
    munmap(map_start, file_size);
    close(fd);
    return -1;
  }

  sieve->fd = fd;
  sieve->map_start = map_start;
  sieve->file_size = file_size;
  sieve->total_elements = total_elements;
  sieve->primes = (uint64_t *)map_start + 1;
  return 0;
}

void small_sieve_close(struct SmallSieve *sieve) {
  if (sieve->map_start != NULL) {
    munmap(sieve->map_start, sieve->file_size);
    sieve->map_start = NULL;
  }
  if (sieve->fd >= 0) {
    close(sieve->fd);
    sieve->fd = -1;
  }
}

int write_large_sieve(const char *path, uint64_t *largeSieve,
                       size_t elementNum) {
  for (size_t i = 0; i < elementNum; i++) {
    largeSieve[i] = htobe64(largeSieve[i]);
  }

  FILE *fout = fopen(path, "wb");
  if (fout == NULL) {
    perror("open output file");
    return -1;
  }
  size_t written = fwrite(largeSieve, sizeof(uint64_t), elementNum, fout);
  fclose(fout);
  if (written != elementNum) {
    fprintf(stderr, "warning: only wrote %zu/%zu large sieve elements\n",
            written, elementNum);
    return -1;
  }
  return 0;
}
