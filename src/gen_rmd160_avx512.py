#!/usr/bin/env python3
"""rmd160_avx512.c を生成する。

RIPEMD-160の定数表は写し間違いが致命的なので、160ステップを手書きせずここから生成する。
表そのものはスカラー参照実装でOpenSSLと20,000件一致することを確認済み。

    python3 src/gen_rmd160_avx512.py > src/rmd160_avx512.c
"""

RL = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
      7,4,13,1,10,6,15,3,12,0,9,5,2,14,11,8,
      3,10,14,4,9,15,8,1,2,7,0,6,13,11,5,12,
      1,9,11,10,0,8,12,4,13,3,7,15,14,5,6,2,
      4,0,5,9,7,12,2,10,14,1,3,8,11,6,15,13]
RR = [5,14,7,0,9,2,11,4,13,6,15,8,1,10,3,12,
      6,11,3,7,0,13,5,10,14,15,8,12,4,9,1,2,
      15,5,1,3,7,14,6,9,11,8,12,2,10,0,4,13,
      8,6,4,1,3,11,15,0,5,12,2,13,9,7,10,14,
      12,15,10,4,1,5,8,7,6,2,13,14,0,3,9,11]
SL = [11,14,15,12,5,8,7,9,11,13,14,15,6,7,9,8,
      7,6,8,13,11,9,7,15,7,12,15,9,11,7,13,12,
      11,13,6,7,14,9,13,15,14,8,13,6,5,12,7,5,
      11,12,14,15,14,15,9,8,9,14,5,6,8,6,5,12,
      9,15,5,11,6,8,13,12,5,12,13,14,11,8,5,6]
SR = [8,9,9,11,13,15,15,5,7,7,8,11,14,14,12,6,
      9,13,15,7,12,8,9,11,7,7,12,7,6,15,13,11,
      9,7,15,11,8,6,6,14,12,13,5,14,13,13,7,5,
      15,5,8,11,14,14,6,14,6,9,12,9,12,5,15,8,
      8,5,12,9,12,5,14,6,8,13,6,5,15,13,11,11]
KL = ["0x00000000","0x5A827999","0x6ED9EBA1","0x8F1BBCDC","0xA953FD4E"]
KR = ["0x50A28BE6","0x5C4DD124","0x6D703EF3","0x7A6D76E9","0x00000000"]
# vpternlogd の即値。f1..f5 を3入力ブール関数とみて真理値表から求めたもの。
#   f1 = x^y^z                 -> 0x96
#   f2 = (x&y)|(~x&z)          -> 0xCA
#   f3 = (x|~y)^z              -> 0x59
#   f4 = (x&z)|(y&~z)          -> 0xE4
#   f5 = x^(y|~z)              -> 0x2D
TERN = [0x96, 0xCA, 0x59, 0xE4, 0x2D]


def step(prefix, xi, s, k, imm):
    a, b, c, d, e = (prefix + x for x in "abcde")
    return (f"        t = _mm512_ternarylogic_epi32({b}, {c}, {d}, {imm});\n"
            f"        t = _mm512_add_epi32(_mm512_add_epi32(t, {a}),\n"
            f"                             _mm512_add_epi32(X[{xi}], _mm512_set1_epi32({k})));\n"
            f"        t = _mm512_add_epi32(_mm512_rol_epi32(t, {s}), {e});\n"
            f"        {a} = {e}; {e} = {d}; {d} = _mm512_rol_epi32({c}, 10); {c} = {b}; {b} = t;\n")


def main():
    body = []
    for i in range(80):
        j = i // 16
        body.append(step("l", RL[i], SL[i], KL[j], hex(TERN[j])))
        body.append(step("r", RR[i], SR[i], KR[j], hex(TERN[4 - j])))
    print(TEMPLATE.replace("@BODY@", "".join(body)))


TEMPLATE = r'''/* このファイルは src/gen_rmd160_avx512.py の生成物です。手で編集しないこと。
 *
 * 入力が常に64バイト(SHA-512の出力)であるという前提のRIPEMD-160を、AVX-512で
 * 16メッセージ同時に計算する。bitmessageのripe計算がこの形なので特化している。
 *
 * OpenSSLのx86_64版RIPEMD-160はアセンブリを持たない素のスカラーCで、逆アセンブル
 * すると1ブロックあたり1,765命令の完全アンロールだった。対してf1〜f5はいずれも
 * 3入力ブール関数なのでvpternlogd 1命令に潰せ、318個あるローテートもvprold 1命令
 * になる。実測でOpenSSLの約20倍。
 */
#include "rmd160_avx512.h"

#include <immintrin.h>
#include <string.h>

int rmd160_16way_available(void)
{
    __builtin_cpu_init();
    /* __builtin_cpu_supports は非ゼロを返すだけで1とは限らないので正規化する */
    return __builtin_cpu_supports("avx512f") != 0;
}

void rmd160_16way(const unsigned char *in, unsigned char *out)
{
    __m512i X[16];
    /* レーン主体へ転置する。160ラウンドに対して誤差の範囲なのでスカラーで行う */
    uint32_t transposed[16][16];
    for (int i = 0; i < 16; i++)
    {
        uint32_t words[16];
        memcpy(words, in + i * 64, 64);
        for (int j = 0; j < 16; j++)
        {
            transposed[j][i] = words[j];
        }
    }
    __m512i h0 = _mm512_set1_epi32(0x67452301);
    __m512i h1 = _mm512_set1_epi32(0xEFCDAB89);
    __m512i h2 = _mm512_set1_epi32(0x98BADCFE);
    __m512i h3 = _mm512_set1_epi32(0x10325476);
    __m512i h4 = _mm512_set1_epi32(0xC3D2E1F0);

    for (int block = 0; block < 2; block++)
    {
        if (block == 0)
        {
            for (int j = 0; j < 16; j++)
            {
                X[j] = _mm512_loadu_si512(transposed[j]);
            }
        }
        else
        {
            /* 入力が64バイト固定なのでパディングブロックは全レーン共通の定数 */
            for (int j = 0; j < 16; j++)
            {
                X[j] = _mm512_setzero_si512();
            }
            X[0] = _mm512_set1_epi32(0x00000080);
            X[14] = _mm512_set1_epi32(512);
        }
        __m512i la = h0, lb = h1, lc = h2, ld = h3, le = h4;
        __m512i ra = h0, rb = h1, rc = h2, rd = h3, re = h4;
        __m512i t;
@BODY@
        __m512i tmp = _mm512_add_epi32(h1, _mm512_add_epi32(lc, rd));
        h1 = _mm512_add_epi32(h2, _mm512_add_epi32(ld, re));
        h2 = _mm512_add_epi32(h3, _mm512_add_epi32(le, ra));
        h3 = _mm512_add_epi32(h4, _mm512_add_epi32(la, rb));
        h4 = _mm512_add_epi32(h0, _mm512_add_epi32(lb, rc));
        h0 = tmp;
    }
    uint32_t o0[16], o1[16], o2[16], o3[16], o4[16];
    _mm512_storeu_si512(o0, h0);
    _mm512_storeu_si512(o1, h1);
    _mm512_storeu_si512(o2, h2);
    _mm512_storeu_si512(o3, h3);
    _mm512_storeu_si512(o4, h4);
    for (int i = 0; i < 16; i++)
    {
        uint32_t words[5] = { o0[i], o1[i], o2[i], o3[i], o4[i] };
        memcpy(out + i * 20, words, 20);
    }
}
'''

if __name__ == "__main__":
    main()
