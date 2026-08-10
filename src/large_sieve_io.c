
#include "large_sieve_io.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <endian.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
  if (fseek(fin, 0, SEEK_END) != 0 || ftell(fin) < 0) {
    perror("fseek/ftell");
    fclose(fin);
    return -1;
  }
  long fsize = ftell(fin);
  if (fseek(fin, 0, SEEK_SET) != 0) {
    perror("fseek");
    fclose(fin);
    return -1;
  }
  char *buf = malloc((size_t)fsize + 1);
  if (buf == NULL) {
    perror("malloc");
    fclose(fin);
    return -1;
  }
  size_t len = fread(buf, 1, (size_t)fsize, fin);
  buf[len] = '\0';
  fclose(fin);

  // 16進数としてa-fを一切含まない場合、10進数のファイルを誤って16進として
  // 読み込んだ可能性が高い(桁数が多いほど偶然a-fを含まない確率は天文学的に低い)
  int has_hex_letter = 0;
  for (size_t i = 0; i < len; i++) {
    char c = buf[i];
    if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
      has_hex_letter = 1;
      break;
    }
  }
  if (!has_hex_letter) {
    fprintf(stderr,
            "warning: %s にa-fが1つも含まれていません。"
            "10進数のファイルを誤って16進として読み込んでいませんか？\n",
            path);
  }

  if (mpz_set_str(base, buf, 16) != 0) {
    fprintf(stderr, "%s の16進数パースに失敗しました\n", path);
    free(buf);
    return -1;
  }
  free(buf);

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
                       size_t elementNum, size_t searchLength) {
  FILE *fout = fopen(path, "wb");
  if (fout == NULL) {
    perror("open output file");
    return -1;
  }

  // small_sieve_openが読む既知素数篩ファイルのヘッダーと対称にする:
  // 先頭8byteにbig-endianでsearchLength(有効なビット数、要素数*64とは限らない
  // 端数を含む)を書く。
  uint64_t header = htobe64((uint64_t)searchLength);
  if (fwrite(&header, sizeof(header), 1, fout) != 1) {
    perror("write large sieve header");
    fclose(fout);
    return -1;
  }

  for (size_t i = 0; i < elementNum; i++) {
    largeSieve[i] = htobe64(largeSieve[i]);
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
