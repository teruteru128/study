/* このファイルは src/gen_rmd160_avx512.py の生成物です。手で編集しないこと。
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
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x50A28BE6)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x5A827999)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x5C4DD124)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 7), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x6ED9EBA1)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x59);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x6D703EF3)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0xe4);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x8F1BBCDC)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0xca);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x7A6D76E9)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[4], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 9), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[12], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[5], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[10], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[7], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[1], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 12), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[2], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[8], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 14), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[14], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[0], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 15), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[6], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 8), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[3], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 13), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[15], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 5), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[9], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;
        t = _mm512_ternarylogic_epi32(lb, lc, ld, 0x2d);
        t = _mm512_add_epi32(_mm512_add_epi32(t, la),
                             _mm512_add_epi32(X[13], _mm512_set1_epi32(0xA953FD4E)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 6), le);
        la = le; le = ld; ld = _mm512_rol_epi32(lc, 10); lc = lb; lb = t;
        t = _mm512_ternarylogic_epi32(rb, rc, rd, 0x96);
        t = _mm512_add_epi32(_mm512_add_epi32(t, ra),
                             _mm512_add_epi32(X[11], _mm512_set1_epi32(0x00000000)));
        t = _mm512_add_epi32(_mm512_rol_epi32(t, 11), re);
        ra = re; re = rd; rd = _mm512_rol_epi32(rc, 10); rc = rb; rb = t;

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

