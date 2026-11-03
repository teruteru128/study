/* bitmessageのripe計算 RIPEMD160(SHA512(署名鍵 || 暗号化鍵)) を16メッセージ同時に
 * 計算するAVX-512実装。Java(FFM)から呼ぶために共有ライブラリとして提供する。
 *
 * どちらも入力長が固定であるという前提に特化しており、パディングブロックが
 * 全レーン共通の定数になるので分岐が消える。
 *   sha512_16way : 130バイト(65バイトの公開鍵2本) -> 64バイト
 *   rmd160_16way :  64バイト(SHA-512の出力)       -> 20バイト
 */
#ifndef BMHASH16_H
#define BMHASH16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** このCPUで rmd160_16way() を呼べるか。0なら呼んではならない。 */
int rmd160_16way_available(void);

/**
 * 64バイトのメッセージ16本から20バイトのダイジェスト16本を計算する。
 *
 * @param in  16 * 64 バイト。連続していること
 * @param out 16 * 20 バイトの書き込み先
 */
void rmd160_16way(const unsigned char *in, unsigned char *out);

/** このCPUで sha512_16way() を呼べるか。0なら呼んではならない。 */
int sha512_16way_available(void);

/**
 * 130バイトのメッセージ16本から64バイトのダイジェスト16本を計算する。
 *
 * @param in  16 * 130 バイト。連続していること
 * @param out 16 * 64 バイトの書き込み先
 */
void sha512_16way(const unsigned char *in, unsigned char *out);

/**
 * 全レーン共通の前半65バイトと、レーンごとに違う後半65バイトからSHA-512を計算する。
 * 呼び出し側が130バイト×16を組み立て直さずに済むので、bitmessageのアドレス探索のように
 * 署名用公開鍵を固定して暗号化用公開鍵だけを変える使い方ではコピーが大きく減る。
 *
 * @param prefix    65バイト。全レーンで共通
 * @param suffixes  65バイト×16。連続していること
 * @param out       16 * 64 バイトの書き込み先
 */
void sha512_16way_prefixed(const unsigned char *prefix, const unsigned char *suffixes,
                           unsigned char *out);

/**
 * ripe16() 用の、署名用公開鍵ごとの事前計算の結果。
 * 中身は実装の都合で変わるので、直接読み書きしないこと。
 */
typedef struct ripe16_sign_ctx
{
    uint64_t opaque[17];
} ripe16_sign_ctx;

/** このCPUで ripe16() を呼べるか(AVX-512FとAVX-512BWが要る)。0なら呼んではならない。 */
int ripe16_available(void);

/**
 * 署名用公開鍵を設定する。同じ鍵を使い続ける間は1回呼べばよい。
 *
 * @param ctx      書き込み先
 * @param sign_key 署名用公開鍵65バイト
 */
void ripe16_set_sign_key(ripe16_sign_ctx *ctx, const unsigned char *sign_key);

/**
 * bitmessageのripe RIPEMD160(SHA512(署名用公開鍵 || 暗号化用公開鍵)) を16本まとめて計算する。
 * sha512_16way_prefixed() + rmd160_16way() と同じ値を返すが、約1.4倍速い。
 *
 * @param ctx      ripe16_set_sign_key() で設定したもの
 * @param enc_keys 暗号化用公開鍵65バイト×16。連続していること。この範囲の外は読まない
 * @param out      16 * 20 バイトの書き込み先
 */
void ripe16(const ripe16_sign_ctx *ctx, const unsigned char *enc_keys, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif
