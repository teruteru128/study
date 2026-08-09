
#ifndef LARGE_SIEVE_IO_H
#define LARGE_SIEVE_IO_H

#include <gmp.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief 16進数テキストで書かれた偶数(基準値)を読み込み、0でなく偶数で
 * あることを検証する。
 *
 * @param path 入力ファイルパス
 * @param base 呼び出し側であらかじめ mpz_init しておくこと
 * @return int 成功時0、失敗時-1(エラーメッセージは出力済み)
 */
int load_even_base(const char *path, mpz_t base);

/**
 * @brief 既知素数ビット篩ファイル。
 * 先頭8byteがbig-endianのbit数ヘッダー、それに続くtotal_elements個の
 * uint64がbig-endianの合成数ビットマップ本体。
 */
struct SmallSieve {
  int fd;
  void *map_start;
  size_t file_size;
  size_t total_elements;
  uint64_t *primes; // big-endian格納のまま。読むときはbe64tohすること
};

/**
 * @brief 既知素数ビット篩ファイルをオープンし、mmapしてヘッダー検証まで
 * 行う。
 *
 * @param path 入力ファイルパス
 * @param sieve 結果の書き込み先
 * @return int 成功時0、失敗時-1(エラーメッセージは出力済み)
 */
int small_sieve_open(const char *path, struct SmallSieve *sieve);

/**
 * @brief small_sieve_openで確保したリソース(mmap, fd)を解放する。
 */
void small_sieve_close(struct SmallSieve *sieve);

/**
 * @brief largeSieveの内容をbig-endianに変換してファイルへ書き出す。
 * 先頭8byteにbig-endianでsearchLength(有効なビット数)のヘッダーを付ける
 * (small_sieve_openが読む既知素数篩ファイルのヘッダーと対称な形式)。
 * 呼び出し後、largeSieveの内容はbig-endianへ書き換わったままになる。
 *
 * @param searchLength 有効なビット数(elementNum*64とは限らない端数を含む)
 * @return int 成功時0、失敗時-1(エラーメッセージは出力済み)
 */
int write_large_sieve(const char *path, uint64_t *largeSieve,
                       size_t elementNum, size_t searchLength);

#endif
