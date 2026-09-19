/* 64バイト入力専用のRIPEMD-160を16メッセージ同時に計算する(AVX-512)。
 * bitmessageのripe計算 RIPEMD160(SHA512(...)) の後段に特化している。 */
#ifndef RMD160_AVX512_H
#define RMD160_AVX512_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * このCPUで rmd160_16way() を呼べるか。0なら呼んではならない。
 */
int rmd160_16way_available(void);

/**
 * 64バイトのメッセージ16本から20バイトのダイジェスト16本を計算する。
 *
 * @param in  16 * 64 バイト。連続していること
 * @param out 16 * 20 バイトの書き込み先
 */
void rmd160_16way(const unsigned char *in, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif
