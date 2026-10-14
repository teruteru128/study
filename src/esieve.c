/* 篩の延長: 既知素数篩の先の素数 p (既定 2^38 付近〜10^14) で、候補 E + step*2 + 1 を割り切るものを探す。
 *
 * 候補は E (2,097,152bit の偶数) + step*2 + 1 (step は最大約671万)。p が step の範囲より十分大きければ、
 * 素数 p ごとに e = E mod p を1回求めるだけで、E + 2*step + 1 ≡ 0 (mod p) となる step がただ1つ決まる。
 * その step が範囲内なら「当たり」(その候補は合成数)。候補ごとにGCDを取る必要は無い。
 *
 * E mod p は、素数 m 個の積の木 + 剰余の木で求める (素数ごとに mpz_fdiv_ui するより約11倍速い。src/esieve_bench.c)。
 * 積の木は偶数が複数あっても1回で済むので、--even は複数指定できる。
 *
 * このプログラムはDBに触らない。当たりを「id p step」の行で --out に書くだけ。
 * 区間(unit)ごとに独立で、完了した unit を --done に追記するので、止めても続きから再開できる。
 *
 * 使い方:
 *   esieve --even even-number-2097152bit-<UUID>.txt [--even ...] [--lo 274877906561] [--hi 1e14]
 *          [--unit 1e10] [--threads 8] [--maxstep 6710886] [--m 32768] [--leaf 32]
 *          [--out hits.tsv] [--done done.tsv] [--check-direct]
 *   esieve --count --lo 1e8 --hi 1e11     (素数生成の検証用。E は使わない)
 * ビルド: cmake --build build-Release --target esieve
 */
#include <errno.h>
#include <gmp.h>
#include <inttypes.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEG_BYTES (1u << 20) /* 1セグメント = 奇数 2^20 個 (数の範囲は 2^21) */

typedef struct {
    char *path;
    uint64_t id;
    mpz_t E;
} even_t;

typedef struct {
    uint32_t even; /* evens[] の添字 */
    uint64_t p, step;
} hit_t;

static struct {
    even_t *evens;
    size_t neven;
    uint64_t lo, hi, unit, maxstep;
    size_t m, leaf;
    int threads, check_direct, count_only;
    const char *out_path, *done_path;
    uint32_t *base; /* 奇素数 (sqrt(hi) まで) */
    size_t nbase;
} cfg;

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ---- E の読み込み ---- */

/* ファイル名の UUID (8-4-4-4-12) の先頭 8-4-4 を16進64bitにしたものがDBの id */
static int id_from_path(const char *path, uint64_t *id)
{
    for (const char *s = path; *s; s++) {
        unsigned a, b, c, d;
        unsigned long long e;
        int n = 0;
        if (sscanf(s, "%8x-%4x-%4x-%4x-%12llx%n", &a, &b, &c, &d, &e, &n) == 5 && n == 36) {
            *id = ((uint64_t)a << 32) | ((uint64_t)b << 16) | c;
            return 0;
        }
    }
    return -1;
}

static int load_even(even_t *ev, const char *path)
{
    ev->path = strdup(path);
    if (id_from_path(path, &ev->id)) {
        fprintf(stderr, "ファイル名からUUIDを取り出せません: %s\n", path);
        return -1;
    }
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "開けません: %s: %s\n", path, strerror(errno));
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *txt = malloc(sz + 1);
    if (!txt || fread(txt, 1, sz, fp) != (size_t)sz) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    txt[sz] = 0;
    /* 末尾の空白・改行を落とす。10進/16進の取り違えは無音で壊れるので、a-fを含まないファイルは警告する */
    while (sz > 0 && (txt[sz - 1] == '\n' || txt[sz - 1] == '\r' || txt[sz - 1] == ' ')) txt[--sz] = 0;
    int has_af = 0;
    for (long i = 0; i < sz; i++)
        if ((txt[i] >= 'a' && txt[i] <= 'f') || (txt[i] >= 'A' && txt[i] <= 'F')) has_af = 1;
    if (!has_af) fprintf(stderr, "警告: %s に a-f が1つも無い。10進数のファイルを16進として読んでいないか確認すること\n", path);
    mpz_init(ev->E);
    if (mpz_set_str(ev->E, txt, 16) != 0) {
        fprintf(stderr, "16進数のパースに失敗: %s\n", path);
        free(txt);
        return -1;
    }
    free(txt);
    if (mpz_odd_p(ev->E)) {
        fprintf(stderr, "エラー: %s は偶数ではない\n", path);
        return -1;
    }
    fprintf(stderr, "even: %s id=%" PRIu64 " (%zu bit)\n", path, ev->id, mpz_sizeinbase(ev->E, 2));
    return 0;
}

/* ---- 素数生成 (分割篩) ---- */

static void make_base_primes(uint64_t limit)
{
    uint64_t n = limit + 1;
    char *c = calloc(n, 1);
    size_t cap = 1024, k = 0;
    cfg.base = malloc(cap * sizeof *cfg.base);
    for (uint64_t i = 3; i < n; i += 2) {
        if (c[i]) continue;
        if (k == cap) { cap *= 2; cfg.base = realloc(cfg.base, cap * sizeof *cfg.base); }
        cfg.base[k++] = (uint32_t)i;
        for (uint64_t j = i * i; j < n; j += 2 * i) c[j] = 1;
    }
    cfg.nbase = k;
    free(c);
}

/* E + 2*step + 1 ≡ 0 (mod p) となる step (0 <= step < p)。e = E mod p */
static uint64_t hit_step(uint64_t e, uint64_t p)
{
    uint64_t t = (p - (e + 1) % p) % p;
    return (uint64_t)(((unsigned __int128)t * ((p + 1) / 2)) % p);
}

/* ---- 積の木 / 剰余の木 ---- */

typedef struct {
    size_t m, leaf, nleaf, nlev;
    uint64_t *pr;
    mpz_t **P, **R;
    size_t *cnt;
    mpz_t t;
    hit_t *hits;
    size_t nhits, caphits;
    uint64_t nprimes, mismatch;
    /* 素数生成 */
    uint64_t *buf;
    size_t bufn;
    uint8_t *seg;
    uint64_t *next;
} work_t;

static void work_init(work_t *w)
{
    memset(w, 0, sizeof *w);
    w->m = cfg.m;
    w->leaf = cfg.leaf;
    w->nleaf = w->m / w->leaf;
    w->nlev = 1;
    for (size_t n = w->nleaf; n > 1; n >>= 1) w->nlev++;
    w->pr = malloc(w->m * sizeof *w->pr);
    w->P = malloc(w->nlev * sizeof *w->P);
    w->R = malloc(w->nlev * sizeof *w->R);
    w->cnt = malloc(w->nlev * sizeof *w->cnt);
    w->cnt[0] = w->nleaf;
    for (size_t l = 0; l < w->nlev; l++) {
        if (l) w->cnt[l] = w->cnt[l - 1] / 2;
        w->P[l] = malloc(w->cnt[l] * sizeof(mpz_t));
        w->R[l] = malloc(w->cnt[l] * sizeof(mpz_t));
        for (size_t i = 0; i < w->cnt[l]; i++) { mpz_init(w->P[l][i]); mpz_init(w->R[l][i]); }
    }
    mpz_init(w->t);
    w->buf = malloc((w->m + 2 * (size_t)SEG_BYTES) * sizeof *w->buf);
    w->seg = malloc(SEG_BYTES);
    w->next = malloc(cfg.nbase * sizeof *w->next);
}

static void hit_push(work_t *w, uint32_t even, uint64_t p, uint64_t step)
{
    if (w->nhits == w->caphits) {
        w->caphits = w->caphits ? w->caphits * 2 : 1024;
        w->hits = realloc(w->hits, w->caphits * sizeof *w->hits);
    }
    w->hits[w->nhits++] = (hit_t){even, p, step};
}

/* pr[0..cnt) (cnt <= m) について、全偶数の E mod p を求めて当たりを集める。足りない分は 1 で埋める */
static void process_chunk(work_t *w, const uint64_t *pr, size_t cnt)
{
    memcpy(w->pr, pr, cnt * sizeof *pr);
    for (size_t i = cnt; i < w->m; i++) w->pr[i] = 1;
    for (size_t g = 0; g < w->nleaf; g++) {
        mpz_set_ui(w->P[0][g], 1);
        for (size_t j = 0; j < w->leaf; j++) {
            uint64_t p = w->pr[g * w->leaf + j];
            if (p > 1) mpz_mul_ui(w->P[0][g], w->P[0][g], p);
        }
    }
    for (size_t l = 1; l < w->nlev; l++)
        for (size_t i = 0; i < w->cnt[l]; i++) mpz_mul(w->P[l][i], w->P[l - 1][2 * i], w->P[l - 1][2 * i + 1]);

    for (size_t k = 0; k < cfg.neven; k++) {
        const mpz_t *E = &cfg.evens[k].E;
        mpz_tdiv_r(w->R[w->nlev - 1][0], *E, w->P[w->nlev - 1][0]);
        for (size_t l = w->nlev - 1; l-- > 0;)
            for (size_t i = 0; i < w->cnt[l]; i++) mpz_tdiv_r(w->R[l][i], w->R[l + 1][i / 2], w->P[l][i]);
        for (size_t i = 0; i < cnt; i++) {
            uint64_t p = w->pr[i];
            uint64_t e = mpz_fdiv_ui(w->R[0][i / w->leaf], p);
            if (cfg.check_direct && e != mpz_fdiv_ui(*E, p)) w->mismatch++;
            uint64_t s = hit_step(e, p);
            if (s > cfg.maxstep) continue;
            /* 木の誤りで偽の当たりを出さないよう、当たりは必ず直接確かめる */
            mpz_add_ui(w->t, *E, 2 * s + 1);
            if (mpz_divisible_ui_p(w->t, p)) hit_push(w, (uint32_t)k, p, s);
            else w->mismatch++;
        }
    }
    w->nprimes += cnt;
}

/* [a, b) の素数を分割篩で生成し、m 個たまるごとに process_chunk (count_only のときは数えるだけ) */
static void process_unit(work_t *w, uint64_t a, uint64_t b)
{
    const size_t L = SEG_BYTES;
    uint64_t S = a & ~1ULL;
    for (size_t i = 0; i < cfg.nbase; i++) {
        uint64_t p = cfg.base[i];
        uint64_t n0 = ((S + 1 + p - 1) / p) * p;
        if (!(n0 & 1)) n0 += p;
        w->next[i] = (n0 - S - 1) / 2;
    }
    w->bufn = 0;
    for (; S < b; S += 2 * (uint64_t)L) {
        memset(w->seg, 0, L);
        for (size_t i = 0; i < cfg.nbase; i++) {
            uint64_t idx = w->next[i], p = cfg.base[i];
            while (idx < L) { w->seg[idx] = 1; idx += p; }
            w->next[i] = idx - L;
        }
        for (size_t j = 0; j < L; j++) {
            if (w->seg[j]) continue;
            uint64_t n = S + 2 * (uint64_t)j + 1;
            if (n < a) continue;
            if (n >= b) break;
            w->buf[w->bufn++] = n;
        }
        if (cfg.count_only) {
            w->nprimes += w->bufn;
            w->bufn = 0;
            continue;
        }
        size_t off = 0;
        while (w->bufn - off >= w->m) { process_chunk(w, w->buf + off, w->m); off += w->m; }
        memmove(w->buf, w->buf + off, (w->bufn - off) * sizeof *w->buf);
        w->bufn -= off;
    }
    if (!cfg.count_only && w->bufn) process_chunk(w, w->buf, w->bufn);
}

/* ---- 再開用: 完了した unit の読み込み ---- */

static uint64_t *load_done(size_t *n)
{
    *n = 0;
    if (!cfg.done_path) return NULL;
    FILE *fp = fopen(cfg.done_path, "r");
    if (!fp) return NULL;
    size_t cap = 256;
    uint64_t *v = malloc(cap * sizeof *v);
    unsigned long long lo, hi, np, nh;
    while (fscanf(fp, "%llu %llu %llu %llu", &lo, &hi, &np, &nh) == 4) {
        if (*n == cap) { cap *= 2; v = realloc(v, cap * sizeof *v); }
        v[(*n)++] = lo;
    }
    fclose(fp);
    return v;
}

static int usage(const char *prog)
{
    fprintf(stderr,
            "使い方: %s --even FILE [--even FILE ...] [--lo N] [--hi N] [--unit N] [--threads N]\n"
            "          [--maxstep N] [--m N] [--leaf N] [--out FILE] [--done FILE] [--check-direct]\n"
            "        %s --count --lo N --hi N [--threads N]   (素数生成の検証)\n",
            prog, prog);
    return 1;
}

int main(int argc, char **argv)
{
    cfg.lo = 274877906561ULL; /* pprimem.c / gcdcheck.c が使っている、既知篩の終端 */
    cfg.hi = 100000000000000ULL;
    cfg.unit = 10000000000ULL;
    cfg.maxstep = 6710886;
    cfg.m = 32768;
    cfg.leaf = 32;
    cfg.threads = 1;
    cfg.out_path = "esieve-hits.tsv";
    cfg.done_path = "esieve-done.tsv";
    cfg.evens = calloc(argc, sizeof *cfg.evens);

    for (int i = 1; i < argc; i++) {
        int hv = i + 1 < argc;
        if (!strcmp(argv[i], "--even") && hv) {
            if (load_even(&cfg.evens[cfg.neven], argv[++i])) return 1;
            cfg.neven++;
        } else if (!strcmp(argv[i], "--lo") && hv) cfg.lo = (uint64_t)strtod(argv[++i], 0);
        else if (!strcmp(argv[i], "--hi") && hv) cfg.hi = (uint64_t)strtod(argv[++i], 0);
        else if (!strcmp(argv[i], "--unit") && hv) cfg.unit = (uint64_t)strtod(argv[++i], 0);
        else if (!strcmp(argv[i], "--threads") && hv) cfg.threads = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--maxstep") && hv) cfg.maxstep = strtoull(argv[++i], 0, 10);
        else if (!strcmp(argv[i], "--m") && hv) cfg.m = strtoul(argv[++i], 0, 10);
        else if (!strcmp(argv[i], "--leaf") && hv) cfg.leaf = strtoul(argv[++i], 0, 10);
        else if (!strcmp(argv[i], "--out") && hv) cfg.out_path = argv[++i];
        else if (!strcmp(argv[i], "--done") && hv) cfg.done_path = argv[++i];
        else if (!strcmp(argv[i], "--check-direct")) cfg.check_direct = 1;
        else if (!strcmp(argv[i], "--count")) cfg.count_only = 1;
        else {
            fprintf(stderr, "不明なオプション、または引数が不足: %s\n", argv[i]);
            return usage(argv[0]);
        }
    }
    if (!cfg.count_only && !cfg.neven) return usage(argv[0]);
    if (cfg.lo >= cfg.hi || cfg.unit == 0 || cfg.threads < 1) return usage(argv[0]);
    size_t nleaf = cfg.leaf ? cfg.m / cfg.leaf : 0;
    if (!cfg.leaf || cfg.m % cfg.leaf || !nleaf || (nleaf & (nleaf - 1))) {
        fprintf(stderr, "m は leaf の倍数で、m/leaf は2のべき乗にすること\n");
        return 1;
    }
    /* p は step の範囲より十分大きい必要がある (hit_step の一意性と、2の逆元) */
    if (!cfg.count_only && cfg.lo <= 2 * cfg.maxstep + 2) {
        fprintf(stderr, "lo は 2*maxstep+2 より大きくすること\n");
        return 1;
    }
    uint64_t sq = (uint64_t)__builtin_sqrtl((long double)cfg.hi) + 2;
    make_base_primes(sq);
    fprintf(stderr, "基礎素数 %zu 個 (〜%" PRIu64 "), 範囲 [%" PRIu64 ", %" PRIu64 "), unit=%" PRIu64
                    ", threads=%d, m=%zu, leaf=%zu\n",
            cfg.nbase, sq, cfg.lo, cfg.hi, cfg.unit, cfg.threads, cfg.m, cfg.leaf);

    uint64_t nunits = (cfg.hi - cfg.lo + cfg.unit - 1) / cfg.unit;
    size_t ndone = 0;
    uint64_t *done = cfg.count_only ? NULL : load_done(&ndone);
    FILE *fout = cfg.count_only ? NULL : fopen(cfg.out_path, "a");
    FILE *fdone = cfg.count_only ? NULL : fopen(cfg.done_path, "a");
    if (!cfg.count_only && (!fout || !fdone)) {
        perror("出力ファイルを開けません");
        return 1;
    }
    uint64_t total_primes = 0, total_hits = 0, mismatch = 0, skipped = 0, finished = 0;
    double t0 = now();

#pragma omp parallel num_threads(cfg.threads)
    {
        work_t w;
        work_init(&w);
#pragma omp for schedule(dynamic, 1)
        for (uint64_t u = 0; u < nunits; u++) {
            uint64_t a = cfg.lo + u * cfg.unit;
            uint64_t b = a + cfg.unit < cfg.hi ? a + cfg.unit : cfg.hi;
            int skip = 0;
            for (size_t k = 0; k < ndone; k++)
                if (done[k] == a) skip = 1;
            if (skip) {
#pragma omp atomic
                skipped++;
                continue;
            }
            w.nprimes = 0;
            w.nhits = 0;
            w.mismatch = 0;
            double ts = now();
            process_unit(&w, a, b);
#pragma omp critical
            {
                if (fout) {
                    for (size_t h = 0; h < w.nhits; h++)
                        fprintf(fout, "%" PRIu64 " %" PRIu64 " %" PRIu64 "\n", cfg.evens[w.hits[h].even].id,
                                w.hits[h].p, w.hits[h].step);
                    fflush(fout);
                    fprintf(fdone, "%" PRIu64 " %" PRIu64 " %" PRIu64 " %zu\n", a, b, w.nprimes, w.nhits);
                    fflush(fdone);
                }
                total_primes += w.nprimes;
                total_hits += w.nhits;
                mismatch += w.mismatch;
                finished++;
                fprintf(stderr, "unit %" PRIu64 "/%" PRIu64 " [%" PRIu64 ", %" PRIu64 ") 素数 %" PRIu64
                                " 個, 当たり %zu 件, %.1f 秒 (%.2f us/素数/スレッド) 累計 %.0f 秒\n",
                        u + 1, nunits, a, b, w.nprimes, w.nhits, now() - ts,
                        w.nprimes ? (now() - ts) / w.nprimes * 1e6 : 0.0, now() - t0);
            }
        }
    }
    double el = now() - t0;
    fprintf(stderr, "完了: unit %" PRIu64 " 個 (再開で飛ばした %" PRIu64 " 個), 素数 %" PRIu64 " 個, 当たり %" PRIu64
                    " 件, 不整合 %" PRIu64 " 件, %.1f 秒\n",
            finished, skipped, total_primes, total_hits, mismatch, el);
    if (cfg.count_only) printf("%" PRIu64 "\n", total_primes);
    return mismatch ? 2 : 0;
}
