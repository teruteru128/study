/*
 * 底2のMiller-Rabin: FLINT fft_small + Montgomery 法。
 *
 * 1ステップ(x <- x^2 R^-1 mod m):
 *   T  = x^2            (2n リム)
 *   q  = lo(T) * ip     (mod B^n。ip = m^-1 mod B^n)
 *   r  = hi(T) - hi(q*m)   (負なら m を足す)
 * lo(T) と lo(q*m) は一致するので、(T - q*m)/B^n は hi 同士の引き算で済む。
 * 乗算は全部 mpn_ctx_mpn_mul。GMP 内部の mpn_binvert だけ使う(ip の計算に1回)。
 */
#include "mr2fs.h"

#include <flint/flint.h>
#include <flint/fft_small.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

/* GMP 6.3.0 内部関数(libgmp が __gmpn_ 名で export している) */
extern void __gmpn_binvert(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_ptr tp);
extern mp_size_t __gmpn_binvert_itch(mp_size_t n);

#define MIN_LIMBS 16

static pthread_key_t ctx_key;
static pthread_once_t ctx_once = PTHREAD_ONCE_INIT;

static void ctx_destroy(void *p)
{
    mpn_ctx_struct *c = p;
    mpn_ctx_clear(c);
    free(c);
}

static void ctx_key_create(void)
{
    pthread_key_create(&ctx_key, ctx_destroy);
}

/* スレッドごとの fft_small コンテキスト */
static mpn_ctx_struct *thread_ctx(void)
{
    pthread_once(&ctx_once, ctx_key_create);
    mpn_ctx_struct *c = pthread_getspecific(ctx_key);
    if (c == NULL) {
        c = malloc(sizeof *c);
        if (c == NULL)
            return NULL;
        mpn_ctx_init(c, UWORD(0x0003f00000000001));
        pthread_setspecific(ctx_key, c);
    }
    return c;
}

int mr2fs_available(void)
{
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
}

/* x <- 2*x mod m (x < m) */
static inline void dbl_mod(mp_ptr x, mp_srcptr m, mp_size_t n)
{
    mp_limb_t cy = mpn_lshift(x, x, n, 1);
    if (cy || mpn_cmp(x, m, n) >= 0)
        mpn_sub_n(x, x, m, n);
}

struct work {
    mpn_ctx_struct *R;
    mp_srcptr m;
    mp_size_t n;
    mp_ptr ip, t, q, qm;
};

/* x <- x^2 R^-1 mod m */
static inline void mont_sqr(struct work *w, mp_ptr x)
{
    mp_size_t n = w->n;
    mpn_ctx_mpn_mul(w->R, w->t, x, n, x, n);
    mpn_ctx_mpn_mul(w->R, w->q, w->t, n, w->ip, n);
    mpn_ctx_mpn_mul(w->R, w->qm, w->q, n, w->m, n);
    if (mpn_sub_n(x, w->t + n, w->qm + n, n))
        mpn_add_n(x, x, w->m, n);
}

/* xout が非NULLなら、d 部分(2^d * R mod m)を終えた直後の残余を書き出す(検証用)。 */
static int run(mpz_srcptr nz, mp_ptr xout)
{
    mp_size_t n = mpz_size(nz);
    mp_srcptr m = mpz_limbs_read(nz);
    if (n < MIN_LIMBS || !(m[0] & 1) || !(m[n - 1] >> 63) || !mr2fs_available())
        return -1;

    struct work w = {.R = thread_ctx(), .m = m, .n = n};
    if (w.R == NULL)
        return -1;
    mp_ptr buf = malloc(10 * n * sizeof(mp_limb_t));  /* x one neg ip (各n) + t q qm (各2n) */
    mp_size_t itch = __gmpn_binvert_itch(n);
    mp_ptr sc = malloc(itch * sizeof(mp_limb_t));
    if (buf == NULL || sc == NULL) {
        free(buf);
        free(sc);
        return -1;
    }
    mp_ptr x = buf, one = x + n, neg = one + n;
    w.ip = neg + n;
    w.t = w.ip + n;
    w.q = w.t + 2 * n;
    w.qm = w.q + 2 * n;
    __gmpn_binvert(w.ip, m, n, sc);
    free(sc);

    /* m-1 = 2^s * d。d のビット列は m-1 の s ビット目以上 */
    mp_limb_t m0 = m[0] - 1;  /* m は奇数なので借りは出ない */
    mp_bitcnt_t s;
    if (m0 != 0) {
        s = __builtin_ctzl(m0);
    } else {
        mp_size_t i = 1;
        while (m[i] == 0)
            i++;
        s = i * GMP_LIMB_BITS + __builtin_ctzl(m[i]);
    }
    mp_bitcnt_t top = n * GMP_LIMB_BITS;  /* m-1 の最上位ビットは n*64-1(最上位ビットが立っているため) */

    /* one = R mod m = B^n - m(m の最上位ビットが立っているので B^n/2 < m < B^n) */
    mpn_com(one, m, n);
    mpn_add_1(one, one, n, 1);
    mpn_sub_n(neg, m, one, n);  /* -R mod m */
    mpn_copyi(x, one, n);
    dbl_mod(x, m, n);           /* d の最上位ビットは1。2R から開始 */

    /* m-1 のビット i (s <= i < top-1) を上位から。ビットは「m[i/64] の (m-1) 版」 */
    for (mp_bitcnt_t i = top - 1; i-- > s;) {
        mont_sqr(&w, x);
        mp_limb_t limb = m[i / GMP_LIMB_BITS];
        if (i / GMP_LIMB_BITS == 0)
            limb = m0;  /* 最下位リムだけ m-1 */
        if ((limb >> (i % GMP_LIMB_BITS)) & 1)
            dbl_mod(x, m, n);
    }

    if (xout != NULL)
        mpn_copyi(xout, x, n);
    int ok = !mpn_cmp(x, one, n) || !mpn_cmp(x, neg, n);
    for (mp_bitcnt_t i = 1; i < s && !ok; i++) {
        mont_sqr(&w, x);
        if (!mpn_cmp(x, neg, n))
            ok = 1;
    }
    free(buf);
    return ok;
}

int mr2fs_strong_base2(mpz_srcptr nz)
{
    return run(nz, NULL);
}

int mr2fs_pow2d_mont(mpz_srcptr nz, mp_ptr out)
{
    return run(nz, out) < 0 ? -1 : 0;
}
