#ifndef MR2FS_H
#define MR2FS_H

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * FLINT fft_small による、底2のMiller-Rabin(強い確率的素数判定)。
 * 乗算はすべて fft_small(AVX2)で行い、Montgomery 法で剰余を取る。
 * 底が2なので「底を掛ける」操作は1ビット左シフトだけで済む。
 *
 * 結果は mpz_powm を使った標準的な実装とビット単位で一致する(src/mr2fs_test.c で検証)。
 *
 * スレッドセーフ。fft_small のコンテキストはスレッドごとに内部で持つ。
 */

/** AVX2 が使えて、このライブラリが動作可能なら 1。そうでなければ 0。 */
int mr2fs_available(void);

/**
 * n が底2の強い確率的素数なら 1、合成数なら 0 を返す。
 * 次の場合は判定せず -1 を返す(呼び出し側は mpz_probab_prime_p 等に任せること):
 *   n が偶数、n が 16 リム未満、n の最上位ビットが立っていない(リム境界に揃っていない)、
 *   AVX2 が使えない。
 */
int mr2fs_strong_base2(mpz_srcptr n);

/**
 * 検証用。d = (n-1) / 2^s(s は n-1 の2の因数の個数)として、2^d * R mod n を out に書き出す
 * (R = B^n。Montgomery 形式のまま)。out は mpz_size(n) リム。
 * 戻り値は mr2fs_strong_base2 の -1 と同じ条件で -1、それ以外は 0。
 */
int mr2fs_pow2d_mont(mpz_srcptr n, mp_ptr out);

#ifdef __cplusplus
}
#endif

#endif
