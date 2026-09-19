#!/usr/bin/env python3
"""sha512_avx512.c を生成する。

SHA-512の定数K[80]とH[8]は数が多く写し間違いが致命的なので、仕様どおりに導出する。
K[i] = i番目の素数の立方根の小数部の上位64bit
H[i] = i番目の素数の平方根の小数部の上位64bit

    python3 src/gen_sha512_avx512.py > src/sha512_avx512.c
"""
import math


def icbrt(n):
    """整数立方根"""
    if n == 0:
        return 0
    x = 1 << ((n.bit_length() + 2) // 3 + 1)
    while True:
        y = (2 * x + n // (x * x)) // 3
        if y >= x:
            return x
        x = y


def primes(count):
    out, candidate = [], 2
    while len(out) < count:
        if all(candidate % p for p in out if p * p <= candidate):
            out.append(candidate)
        candidate += 1
    return out


def main():
    ps = primes(80)
    k = [icbrt(p << 192) - (icbrt(p) << 64) for p in ps]
    h = [math.isqrt(p << 128) - (math.isqrt(p) << 64) for p in ps[:8]]
    # 導出が正しいことを既知値で確認してから吐く
    assert k[0] == 0x428A2F98D728AE22 and k[79] == 0x6C44198C4A475817
    assert h[0] == 0x6A09E667F3BCC908 and h[7] == 0x5BE0CD19137E2179

    kconst = "".join("    0x%016xULL,\n" % x for x in k)

    def hinit(sfx):
        return "".join(
            "    __m512i h%d%s = _mm512_set1_epi64((long long)0x%016xULL);\n" % (i, sfx, h[i])
            for i in range(8))

    def round_body(s):
        return f"""            __m512i t1{s} = _mm512_add_epi64(
                _mm512_add_epi64(_mm512_add_epi64(hh{s}, BIGSIG1(e{s})), CH(e{s}, f{s}, g{s})),
                _mm512_add_epi64(kt, w{s}[t]));
            __m512i t2{s} = _mm512_add_epi64(BIGSIG0(a{s}), MAJ(a{s}, b{s}, cc{s}));
            hh{s} = g{s}; g{s} = f{s}; f{s} = e{s}; e{s} = _mm512_add_epi64(d{s}, t1{s});
            d{s} = cc{s}; cc{s} = b{s}; b{s} = a{s}; a{s} = _mm512_add_epi64(t1{s}, t2{s});
"""

    def tail(s, off):
        return "".join(
            "    h%d%s = _mm512_add_epi64(h%d%s, %s);\n" % (i, s, i, s, v + s)
            for i, v in enumerate(["a", "b", "cc", "d", "e", "f", "g", "hh"]))

    print(TEMPLATE
          .replace("@K@", kconst)
          .replace("@HINIT_A@", hinit("A"))
          .replace("@HINIT_B@", hinit("B"))
          .replace("@ROUND_A@", round_body("A"))
          .replace("@ROUND_B@", round_body("B")))


TEMPLATE = r'''/* このファイルは src/gen_sha512_avx512.py の生成物です。手で編集しないこと。
 *
 * 入力が常に130バイト(65バイトの公開鍵2本)であるという前提のSHA-512を、AVX-512で
 * 16メッセージ同時に計算する。bitmessageのripe計算の前段がこの形。
 *
 * zmmは512bitなので64bit値は8レーンしか入らない。16レーンにするため8レーン版を
 * 2つ(A/B)交互に走らせている。命令数はちょうど2倍になるが速度は1.14倍しか
 * 上がらない(既にスループット律速で、ILPの余地が1割程度しか無いため)。それでも
 * 呼び出し側のバッチ幅16とそのまま噛み合うので16レーンを採っている。
 *
 * ChとMajは3入力ブール関数なのでvpternlogq 1命令、ROR3つのxorも同様。
 */
#include "bmhash16.h"

#include <immintrin.h>
#include <string.h>

#define MSG_LEN 130

static const uint64_t K[80] = {
@K@};

/* Ch(e,f,g) = (e&f)^(~e&g) -> 0xCA,  Maj(a,b,c) = (a&b)^(a&c)^(b&c) -> 0xE8
 * 3値のxor -> 0x96 */
#define CH(e, f, g) _mm512_ternarylogic_epi64(e, f, g, 0xCA)
#define MAJ(a, b, c) _mm512_ternarylogic_epi64(a, b, c, 0xE8)
#define XOR3(x, y, z) _mm512_ternarylogic_epi64(x, y, z, 0x96)
#define BIGSIG0(a) XOR3(_mm512_ror_epi64(a, 28), _mm512_ror_epi64(a, 34), _mm512_ror_epi64(a, 39))
#define BIGSIG1(e) XOR3(_mm512_ror_epi64(e, 14), _mm512_ror_epi64(e, 18), _mm512_ror_epi64(e, 41))
#define SIG0(x) XOR3(_mm512_ror_epi64(x, 1), _mm512_ror_epi64(x, 8), _mm512_srli_epi64(x, 7))
#define SIG1(x) XOR3(_mm512_ror_epi64(x, 19), _mm512_ror_epi64(x, 61), _mm512_srli_epi64(x, 6))

int sha512_16way_available(void)
{
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512f") != 0;
}

void sha512_16way(const unsigned char *in, unsigned char *out)
{
    /* 入力長が固定なのでパディングは毎回同じ形になる */
    unsigned char buf[16][256];
    memset(buf, 0, sizeof buf);
    for (int i = 0; i < 16; i++)
    {
        memcpy(buf[i], in + i * MSG_LEN, MSG_LEN);
        buf[i][MSG_LEN] = 0x80;
        uint64_t bits = (uint64_t)MSG_LEN * 8;
        for (int b = 0; b < 8; b++)
        {
            buf[i][255 - b] = (unsigned char)(bits >> (8 * b));
        }
    }
@HINIT_A@@HINIT_B@
    for (int block = 0; block < 2; block++)
    {
        __m512i wA[80], wB[80];
        for (int t = 0; t < 16; t++)
        {
            uint64_t la[8], lb[8];
            for (int i = 0; i < 8; i++)
            {
                uint64_t v = 0, u = 0;
                for (int b = 0; b < 8; b++)
                {
                    v = (v << 8) | buf[i][block * 128 + t * 8 + b];
                    u = (u << 8) | buf[i + 8][block * 128 + t * 8 + b];
                }
                la[i] = v;
                lb[i] = u;
            }
            wA[t] = _mm512_loadu_si512(la);
            wB[t] = _mm512_loadu_si512(lb);
        }
        for (int t = 16; t < 80; t++)
        {
            wA[t] = _mm512_add_epi64(_mm512_add_epi64(wA[t - 16], SIG0(wA[t - 15])),
                                     _mm512_add_epi64(wA[t - 7], SIG1(wA[t - 2])));
            wB[t] = _mm512_add_epi64(_mm512_add_epi64(wB[t - 16], SIG0(wB[t - 15])),
                                     _mm512_add_epi64(wB[t - 7], SIG1(wB[t - 2])));
        }
        __m512i aA = h0A, bA = h1A, ccA = h2A, dA = h3A, eA = h4A, fA = h5A, gA = h6A, hhA = h7A;
        __m512i aB = h0B, bB = h1B, ccB = h2B, dB = h3B, eB = h4B, fB = h5B, gB = h6B, hhB = h7B;
        for (int t = 0; t < 80; t++)
        {
            __m512i kt = _mm512_set1_epi64((long long)K[t]);
@ROUND_A@@ROUND_B@        }
        h0A = _mm512_add_epi64(h0A, aA); h1A = _mm512_add_epi64(h1A, bA);
        h2A = _mm512_add_epi64(h2A, ccA); h3A = _mm512_add_epi64(h3A, dA);
        h4A = _mm512_add_epi64(h4A, eA); h5A = _mm512_add_epi64(h5A, fA);
        h6A = _mm512_add_epi64(h6A, gA); h7A = _mm512_add_epi64(h7A, hhA);
        h0B = _mm512_add_epi64(h0B, aB); h1B = _mm512_add_epi64(h1B, bB);
        h2B = _mm512_add_epi64(h2B, ccB); h3B = _mm512_add_epi64(h3B, dB);
        h4B = _mm512_add_epi64(h4B, eB); h5B = _mm512_add_epi64(h5B, fB);
        h6B = _mm512_add_epi64(h6B, gB); h7B = _mm512_add_epi64(h7B, hhB);
    }
    __m512i hsA[8] = { h0A, h1A, h2A, h3A, h4A, h5A, h6A, h7A };
    __m512i hsB[8] = { h0B, h1B, h2B, h3B, h4B, h5B, h6B, h7B };
    for (int j = 0; j < 8; j++)
    {
        uint64_t la[8], lb[8];
        _mm512_storeu_si512(la, hsA[j]);
        _mm512_storeu_si512(lb, hsB[j]);
        for (int i = 0; i < 8; i++)
        {
            for (int b = 0; b < 8; b++)
            {
                out[i * 64 + j * 8 + b] = (unsigned char)(la[i] >> (56 - 8 * b));
                out[(i + 8) * 64 + j * 8 + b] = (unsigned char)(lb[i] >> (56 - 8 * b));
            }
        }
    }
}
'''

if __name__ == "__main__":
    main()
