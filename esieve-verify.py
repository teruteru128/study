#!/usr/bin/env python3
"""esieve の当たり (id p step の行) を、GMPを使わない独立した計算で検算する。

  esieve-verify.py --even even-number-...-<UUID>.txt [--even ...] hits.tsv [hits2.tsv ...]

各行について次を確かめる (どれか1つでも崩れたら終了コード1。DBには反映しないこと):
  - id が --even で与えた偶数のどれかに対応する
  - p が奇数で 1 < p、step が 0 <= step <= maxstep
  - (E + 2*step + 1) mod p == 0   (候補が p で割り切れる = 合成数)
  - p < E + 2*step + 1            (候補自身ではない)
誤検出 (割り切れないのに合成数扱い) は本物の素数候補を捨てることになるので、
esieve-apply.sh --apply の前に必ずこれを通すこと。見逃しは効率が落ちるだけで害は無い。
"""
import argparse
import re
import sys


def uuid_id(path):
    m = re.search(r"([0-9a-fA-F]{8})-([0-9a-fA-F]{4})-([0-9a-fA-F]{4})-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}", path)
    if not m:
        sys.exit("ファイル名からUUIDを取り出せません: " + path)
    return int(m.group(1) + m.group(2) + m.group(3), 16)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--even", action="append", required=True)
    ap.add_argument("--maxstep", type=int, default=6710886)
    ap.add_argument("hits", nargs="+")
    a = ap.parse_args()

    evens = {}
    for path in a.even:
        text = open(path).read().strip()
        evens[uuid_id(path)] = int(text, 16)
        if evens[uuid_id(path)] % 2:
            sys.exit("偶数ではありません: " + path)

    total = bad = 0
    per_id = {}
    for hp in a.hits:
        for ln, line in enumerate(open(hp), 1):
            f = line.split()
            if len(f) != 3:
                print(f"{hp}:{ln}: 形式不正: {line.strip()!r}")
                bad += 1
                continue
            i, p, s = map(int, f)
            total += 1
            per_id[i] = per_id.get(i, 0) + 1
            e = evens.get(i)
            err = None
            if e is None:
                err = "未知のid"
            elif p < 3 or p % 2 == 0:
                err = "pが奇数でない"
            elif not 0 <= s <= a.maxstep:
                err = "stepが範囲外"
            else:
                n = e + 2 * s + 1
                if n % p != 0:
                    err = "割り切れない"
                elif p >= n:
                    err = "p が候補自身"
            if err:
                bad += 1
                if bad <= 20:
                    print(f"{hp}:{ln}: {err}: id={i} p={p} step={s}")
    print(f"検算した当たり {total} 件 (id別: {per_id}), 不正 {bad} 件")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
