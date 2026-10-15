#!/usr/bin/env python3
"""esieve と esieve-verify.py のテスト。

  esieve_test.py --esieve <esieveのバイナリ> --verify <esieve-verify.py>

本番の偶数ファイルは使わず、テスト内で人工的に作る。E' は CRT で
「E' + 2*step + 1 が指定した素数 p で割り切れる」ように作るので、出るべき当たりが事前に分かる。

  1. 素数の個数: 既知の π(x) と一致する (unit の境界が奇数でも、スレッド数が違っても)
  2. 植え込んだ (p, step) を全部見つける。step の上限 (maxstep) を超えるものは出さない。
     出た当たりは全部、独立に検算しても正しい (偽の当たりが無い)
  3. 再開: 同じ引数でもう一度流すと、完了した unit を飛ばし、当たりを重複して書かない
  4. スレッド数1と4で当たりの集合が同じ
  5. esieve-verify.py は偽の当たり (割り切れない、p が偶数) を検出して終了コード1を返す
  6. 不正な引数は終了コード1
"""
import argparse
import os
import random
import subprocess
import sys
import tempfile

MAXSTEP = 6710886
LO = 280_000_000_000          # 2*maxstep+2 より十分大きい
WIDTH = 3_000_000
BITS = 2_097_152

failures = 0
checks = 0


def check(cond, msg):
    global failures, checks
    checks += 1
    if not cond:
        failures += 1
        print("NG:", msg)


def is_prime(n):
    if n < 2:
        return False
    for p in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        if n % p == 0:
            return n == p
    d, s = n - 1, 0
    while d % 2 == 0:
        d //= 2
        s += 1
    for a in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(s - 1):
            x = x * x % n
            if x == n - 1:
                break
        else:
            return False
    return True


def next_prime(n):
    while not is_prime(n):
        n += 1
    return n


def make_even(rng, plants):
    """E' + 2*step + 1 ≡ 0 (mod p) を plants=[(p, step), ...] 全部について満たす偶数を作る"""
    e = rng.getrandbits(BITS) | (1 << (BITS - 1))
    r, m = 0, 2                                   # E' ≡ 0 (mod 2)
    for p, s in plants:
        t = (-(2 * s + 1)) % p
        r += m * (((t - r) * pow(m, -1, p)) % p)
        m *= p
    return e - ((e - r) % m)


def write_even(path, e):
    assert e % 2 == 0 and e.bit_length() == BITS
    with open(path, "w") as f:
        f.write("%x\n" % e)


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def read_hits(path):
    return {tuple(map(int, line.split())) for line in open(path)} if os.path.exists(path) else set()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--esieve", required=True)
    ap.add_argument("--verify", required=True)
    a = ap.parse_args()

    rng = random.Random(20261016)
    with tempfile.TemporaryDirectory() as tmp:
        os.chdir(tmp)

        # ---- 1. 素数の個数 ----
        for lo, hi, unit, threads, want in (
            (10**8, 2 * 10**8, 7 * 10**7, 2, 11078937 - 5761455),          # π(2e8) - π(1e8)
            (10**8, 10**9, 33_333_333, 4, 50847534 - 5761455),             # π(1e9) - π(1e8)、unit の境界が奇数
            (10**8, 10**9, 10**9, 1, 50847534 - 5761455),                  # 1 unit、1スレッド
        ):
            r = run([a.esieve, "--count", "--lo", str(lo), "--hi", str(hi), "--unit", str(unit), "--threads", str(threads)])
            got = int(r.stdout.strip() or -1)
            check(r.returncode == 0 and got == want, f"素数の個数 [{lo},{hi}) unit={unit} threads={threads}: {got} (期待 {want})")

        # ---- 2. 植え込み ----
        window_primes = []
        n = LO + 1000
        for _ in range(8):
            n = next_prime(n + rng.randrange(100_000, 300_000))
            window_primes.append(n)
            n += 1
        assert window_primes[-1] < LO + WIDTH
        steps = [0, 1, 2, MAXSTEP - 1, MAXSTEP, 123456, MAXSTEP + 1, MAXSTEP + 1000]
        plants1 = list(zip(window_primes[:4], steps[:4]))
        plants1 += [(window_primes[4], steps[4])]
        plants2 = [(window_primes[5], steps[5]),
                   (window_primes[6], steps[6]),      # 上限を超える: 出てはいけない
                   (window_primes[7], steps[7])]      # 上限を超える: 出てはいけない
        files = {
            "even-number-2097152bit-aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.txt": (0xAAAAAAAABBBBCCCC, plants1),
            "even-number-2097152bit-11111111-2222-3333-4444-555555555555.txt": (0x1111111122223333, plants2),
        }
        evens = []
        for path, (_, plants) in files.items():
            write_even(path, make_even(rng, plants))
            evens += ["--even", path]
        expected = set()
        absent = set()
        for _, (ident, plants) in files.items():
            for p, s in plants:
                (expected if s <= MAXSTEP else absent).add((ident, p, s))

        common = evens + ["--lo", str(LO), "--hi", str(LO + WIDTH), "--unit", "1000000", "--check-direct"]
        r = run([a.esieve] + common + ["--threads", "4", "--out", "hits4.tsv", "--done", "done4.tsv"])
        check(r.returncode == 0, f"esieve が失敗: rc={r.returncode}\n{r.stderr[-500:]}")
        got = read_hits("hits4.tsv")
        check(expected <= got, f"植え込みを見逃した: {sorted(expected - got)}")
        check(not (absent & got), f"maxstep を超える当たりを出した: {sorted(absent & got)}")
        check("不整合 0 件" in r.stderr, "木と直接法が不整合 (--check-direct)")
        check({i for i, _, _ in got} <= {0xAAAAAAAABBBBCCCC, 0x1111111122223333}, "未知の id の当たり")
        # 偽の当たりが無い: 出た当たりを全部、独立に検算する
        v = run([sys.executable, "-I", a.verify] + evens + ["hits4.tsv"])
        check(v.returncode == 0, f"検算が不正を検出: {v.stdout[-300:]}")

        # ---- 3. 再開 ----
        before = open("hits4.tsv").read()
        r2 = run([a.esieve] + common + ["--threads", "4", "--out", "hits4.tsv", "--done", "done4.tsv"])
        check("再開で飛ばした 3 個" in r2.stderr, f"再開で unit を飛ばさない: {r2.stderr[-200:]}")
        check(open("hits4.tsv").read() == before, "再開で当たりが重複して書かれた")

        # ---- 4. スレッド数に依存しない ----
        r3 = run([a.esieve] + common + ["--threads", "1", "--out", "hits1.tsv", "--done", "done1.tsv"])
        check(r3.returncode == 0 and read_hits("hits1.tsv") == got, "スレッド数1と4で当たりの集合が違う")

        # ---- 5. 検算スクリプトが偽の当たりを検出する ----
        ident, p = 0xAAAAAAAABBBBCCCC, window_primes[0]
        for bad, why in ((f"{ident} {p} 5\n", "割り切れない"), (f"{ident} {p + 1} 0\n", "p が偶数"),
                         (f"{ident} {p} {MAXSTEP + 1}\n", "step が範囲外"), ("999 3 3\n", "未知の id")):
            with open("bad.tsv", "w") as f:
                f.write(open("hits4.tsv").read() + bad)
            v = run([sys.executable, "-I", a.verify] + evens + ["bad.tsv"])
            check(v.returncode == 1, f"検算が偽の当たりを見逃した ({why})")

        # ---- 6. 不正な引数 ----
        check(run([a.esieve, "--count", "--lo", "100", "--hi", "10"]).returncode == 1, "lo >= hi がエラーにならない")
        check(run([a.esieve]).returncode == 1, "引数なしがエラーにならない")
        check(run([a.esieve] + evens + ["--lo", "1000", "--hi", "2000"]).returncode == 1,
              "lo が小さすぎる (p が step の範囲に入る) のにエラーにならない")
        check(run([a.esieve, "--bogus"]).returncode == 1, "不明なオプションがエラーにならない")

    print(f"esieve_test: {checks} 件中 {failures} 件失敗")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
