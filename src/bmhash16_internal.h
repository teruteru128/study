/* bmhash16ライブラリ内部でだけ使う宣言。公開ヘッダ(bmhash16.h)には載せない。 */
#ifndef BMHASH16_INTERNAL_H
#define BMHASH16_INTERNAL_H

#include <immintrin.h>

/**
 * rmd160_16way() の本体。入力をレーン主体のベクトルで受け取る。
 * in[j] のレーン i が、メッセージ i の j 番目の32bitワード(リトルエンディアン)。
 * SHA-512の出力をバイト列に戻さずに渡すために分けてある。
 */
__attribute__((visibility("hidden")))
void rmd160_16way_lanes(const __m512i *in, unsigned char *out);

#endif
