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
          .replace("@H@", "".join("    0x%016xULL,\n" % x for x in h))
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
#include "bmhash16_internal.h"

#include <immintrin.h>
#include <string.h>

#define MSG_LEN 130
#define PREFIX_LEN 65
#define SUFFIX_LEN 65

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

/* パディング済みの2ブロックを16レーンで処理する。入口はbufの組み立て方だけが違う */
static void sha512_16way_blocks(const unsigned char buf[16][256], unsigned char *out)
{
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

/* 入力長が固定なのでパディングは毎回同じ形になる */
static void pad(unsigned char buf[16][256])
{
    uint64_t bits = (uint64_t)MSG_LEN * 8;
    for (int i = 0; i < 16; i++)
    {
        buf[i][MSG_LEN] = 0x80;
        for (int b = 0; b < 8; b++)
        {
            buf[i][255 - b] = (unsigned char)(bits >> (8 * b));
        }
    }
}

void sha512_16way(const unsigned char *in, unsigned char *out)
{
    unsigned char buf[16][256];
    memset(buf, 0, sizeof buf);
    for (int i = 0; i < 16; i++)
    {
        memcpy(buf[i], in + i * MSG_LEN, MSG_LEN);
    }
    pad(buf);
    sha512_16way_blocks(buf, out);
}

void sha512_16way_prefixed(const unsigned char *prefix, const unsigned char *suffixes,
                           unsigned char *out)
{
    /* 全レーンで共通の前半(署名用公開鍵)と、レーンごとに違う後半(暗号化用公開鍵)。
     * 呼び出し側が130バイト x 16 を組み立て直す必要がなくなる。 */
    unsigned char buf[16][256];
    memset(buf, 0, sizeof buf);
    for (int i = 0; i < 16; i++)
    {
        memcpy(buf[i], prefix, PREFIX_LEN);
        memcpy(buf[i] + PREFIX_LEN, suffixes + i * SUFFIX_LEN, SUFFIX_LEN);
    }
    pad(buf);
    sha512_16way_blocks(buf, out);
}

/* ---- ripe16: RIPEMD160(SHA512(署名用公開鍵 || 暗号化用公開鍵)) を一体で計算する ----
 *
 * bitmessageのアドレス探索は、署名用公開鍵を固定して暗号化用公開鍵だけを変えていく。
 * sha512_16way_prefixed() + rmd160_16way() との違いは次の3つ。2026-11-03の試作の実測
 * (Ryzen 7 8745H、16スレッド)で、5.1〜5.35e7組/秒が7.48e7組/秒(約1.4倍)になった。
 *  1. 入力の組み立て: 1バイトずつシフトして詰めるのをやめ、gatherで8バイトずつ読んで
 *     vpshufbでビッグエンディアンに直す。効果の大半(約+34%)はこれ
 *  2. 1ブロック目の先頭64バイト(W0〜W7)は署名用公開鍵だけで決まるので、最初の8ラウンドを
 *     ripe16_set_sign_key() で1回だけ計算しておく(約+3%)
 *  3. SHA-512の出力をバイト列に戻さず、レジスタ上でRIPEMD-160の入力の並びへ組み替える(約+5%)
 *
 * 512bitのvpshufbにAVX-512BWが要るので、ripe16_available() はavx512bwも確かめる。
 */
typedef struct
{
    uint64_t w[8];   /* 1ブロック目のW0〜W7 */
    uint64_t st[8];  /* 8ラウンド後の作業変数 a〜h */
    uint64_t sign64; /* 署名用公開鍵の65バイト目 << 56。W8の最上位バイトになる */
} sign_ctx_impl;

_Static_assert(sizeof(sign_ctx_impl) <= sizeof(ripe16_sign_ctx), "ripe16_sign_ctxが小さすぎる");

static const uint64_t H[8] = {
@H@};

/* 64bit/32bitワードごとのバイト順反転(vpshufbは128bitレーン内で効く) */
static const unsigned char BSWAP64[64] = {
#define BS64_ 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
    BS64_, BS64_, BS64_, BS64_
#undef BS64_
};
static const unsigned char BSWAP32[64] = {
#define BS32_ 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12
    BS32_, BS32_, BS32_, BS32_
#undef BS32_
};

static inline uint64_t ror64(uint64_t x, int n)
{
    return (x >> n) | (x << (64 - n));
}

int ripe16_available(void)
{
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx512f") != 0 && __builtin_cpu_supports("avx512bw") != 0;
}

void ripe16_set_sign_key(ripe16_sign_ctx *ctx, const unsigned char *sign_key)
{
    sign_ctx_impl c;
    for (int t = 0; t < 8; t++)
    {
        uint64_t v = 0;
        for (int b = 0; b < 8; b++)
        {
            v = (v << 8) | sign_key[t * 8 + b];
        }
        c.w[t] = v;
    }
    c.sign64 = (uint64_t)sign_key[64] << 56;
    uint64_t a = H[0], b = H[1], cc = H[2], d = H[3], e = H[4], f = H[5], g = H[6], hh = H[7];
    for (int t = 0; t < 8; t++)
    {
        uint64_t t1 = hh + (ror64(e, 14) ^ ror64(e, 18) ^ ror64(e, 41)) + ((e & f) ^ (~e & g))
                      + K[t] + c.w[t];
        uint64_t t2 = (ror64(a, 28) ^ ror64(a, 34) ^ ror64(a, 39)) + ((a & b) ^ (a & cc) ^ (b & cc));
        hh = g; g = f; f = e; e = d + t1;
        d = cc; cc = b; b = a; a = t1 + t2;
    }
    const uint64_t st[8] = { a, b, cc, d, e, f, g, hh };
    memcpy(c.st, st, sizeof st);
    memset(ctx, 0, sizeof *ctx);
    memcpy(ctx, &c, sizeof c);
}

/* 暗号化用公開鍵(65バイト刻みで16本)の off バイト目から8バイトを、8レーンぶん
 * ビッグエンディアンの64bitワードとして読む */
#define LOAD_BE(base, vidx, off, bswap) \
    _mm512_shuffle_epi8(_mm512_i64gather_epi64((vidx), (const void *)((base) + (off)), 1), (bswap))

__attribute__((target("avx512f,avx512bw")))
void ripe16(const ripe16_sign_ctx *ctx, const unsigned char *enc_keys, unsigned char *out)
{
    sign_ctx_impl c;
    memcpy(&c, ctx, sizeof c);
    const __m512i bswap64 = _mm512_loadu_si512(BSWAP64);
    /* レーン0〜7(A)と8〜15(B)の鍵の先頭位置 */
    const __m512i vidx[2] = { _mm512_setr_epi64(0, 65, 130, 195, 260, 325, 390, 455),
                              _mm512_setr_epi64(520, 585, 650, 715, 780, 845, 910, 975) };
    /* メッセージは 署名鍵65 || 暗号化鍵65 || 0x80 || 0... || 長さ1040bit の256バイト。
     * 暗号化鍵の中のバイト位置で言うと、1ブロック目の W8 = 署名鍵[64] || 暗号化鍵[0..6]、
     * W9〜W15 = 暗号化鍵[7..62]、2ブロック目の W0 = 暗号化鍵[63..64] || 0x80 || 0...、
     * W1〜W14 = 0、W15 = 1040。読むのは各鍵の0〜64バイト目だけなので、
     * enc_keys の 16*65 バイトの外には触れない(最後は975+57+7 = 1039バイト目) */
    __m512i w[2][80];
    __m512i b2w0[2];
    for (int h = 0; h < 2; h++)
    {
        for (int t = 0; t < 8; t++)
        {
            w[h][t] = _mm512_set1_epi64((long long)c.w[t]);
        }
        w[h][8] = _mm512_or_si512(_mm512_set1_epi64((long long)c.sign64),
                                  _mm512_srli_epi64(LOAD_BE(enc_keys, vidx[h], 0, bswap64), 8));
        for (int k = 0; k < 7; k++)
        {
            w[h][9 + k] = LOAD_BE(enc_keys, vidx[h], 7 + 8 * k, bswap64);
        }
        b2w0[h] = _mm512_or_si512(_mm512_slli_epi64(LOAD_BE(enc_keys, vidx[h], 57, bswap64), 48),
                                  _mm512_set1_epi64((long long)(0x80ULL << 40)));
    }
    __m512i hs[2][8];
    for (int block = 0; block < 2; block++)
    {
        if (block == 1)
        {
            for (int h = 0; h < 2; h++)
            {
                w[h][0] = b2w0[h];
                for (int t = 1; t < 15; t++)
                {
                    w[h][t] = _mm512_setzero_si512();
                }
                w[h][15] = _mm512_set1_epi64((long long)MSG_LEN * 8);
            }
        }
        for (int t = 16; t < 80; t++)
        {
            for (int h = 0; h < 2; h++)
            {
                w[h][t] = _mm512_add_epi64(_mm512_add_epi64(w[h][t - 16], SIG0(w[h][t - 15])),
                                           _mm512_add_epi64(w[h][t - 7], SIG1(w[h][t - 2])));
            }
        }
        __m512i v[2][8];
        for (int h = 0; h < 2; h++)
        {
            for (int i = 0; i < 8; i++)
            {
                v[h][i] = block == 0 ? _mm512_set1_epi64((long long)c.st[i]) : hs[h][i];
            }
        }
        /* 1ブロック目の最初の8ラウンドは ripe16_set_sign_key() で済んでいる */
        for (int t = block == 0 ? 8 : 0; t < 80; t++)
        {
            __m512i kt = _mm512_set1_epi64((long long)K[t]);
            /* A/Bを交互に回して命令レベルの並列性を稼ぐ */
            for (int h = 0; h < 2; h++)
            {
                __m512i t1 = _mm512_add_epi64(
                    _mm512_add_epi64(_mm512_add_epi64(v[h][7], BIGSIG1(v[h][4])),
                                     CH(v[h][4], v[h][5], v[h][6])),
                    _mm512_add_epi64(kt, w[h][t]));
                __m512i t2 = _mm512_add_epi64(BIGSIG0(v[h][0]), MAJ(v[h][0], v[h][1], v[h][2]));
                v[h][7] = v[h][6]; v[h][6] = v[h][5]; v[h][5] = v[h][4];
                v[h][4] = _mm512_add_epi64(v[h][3], t1);
                v[h][3] = v[h][2]; v[h][2] = v[h][1]; v[h][1] = v[h][0];
                v[h][0] = _mm512_add_epi64(t1, t2);
            }
        }
        for (int h = 0; h < 2; h++)
        {
            for (int i = 0; i < 8; i++)
            {
                hs[h][i] = _mm512_add_epi64(
                    block == 0 ? _mm512_set1_epi64((long long)H[i]) : hs[h][i], v[h][i]);
            }
        }
    }
    /* ダイジェストのワードjはビッグエンディアンの64bit。RIPEMD-160はこれを
     * リトルエンディアンの32bitワード2つ(上位半分のバイト列、下位半分のバイト列)として読む。
     * A(レーン0〜7)とB(8〜15)から上位/下位の32bitを拾って16レーンに並べ、バイト順を反転する */
    const __m512i bswap32 = _mm512_loadu_si512(BSWAP32);
    const __m512i hi = _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
    const __m512i lo = _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
    __m512i X[16];
    for (int j = 0; j < 8; j++)
    {
        X[2 * j] = _mm512_shuffle_epi8(_mm512_permutex2var_epi32(hs[0][j], hi, hs[1][j]), bswap32);
        X[2 * j + 1] =
            _mm512_shuffle_epi8(_mm512_permutex2var_epi32(hs[0][j], lo, hs[1][j]), bswap32);
    }
    rmd160_16way_lanes(X, out);
}
'''

if __name__ == "__main__":
    main()
