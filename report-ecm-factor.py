#!/usr/bin/env python3
"""GMP-ECMの出力から見つかった因数を読み取り、factordbへ報告する。

factordbへの報告は2段階に分かれている。

1. 因数そのものの登録          POST reportfactor.php  (number|id, factor)
2. 見つけた方法の登録          GET  index.php?id=<親>&open=ecm&cofac=<因数>
                                     &method=<ecm|ecme|M1|P1>&b1=&b2=&sigma=

2の方は `index.php` 内で ajax.js が `frame_ecm.php?id=<親>` を読み込んで
表示しているフォームと同じもので、実体は素直なGETフォーム。ブラウザは要らない。

曲線タイプは GMP-ECM の `sigma=P:S` の P (= -param の値) で決まる。
  P=0 → Suyama/Montgomery → method=ecm
  P>0 → Edwards           → method=ecme
sigmaとして送るのは S の部分のみ。
"""
import argparse
import os
import re
import sys
import time

import requests

FDB = "https://factordb.com"
SESSION_ENV = "FDB_SESSION_ID"

RE_INPUT = re.compile(r"^Input number is (.+?) \((\d+) digits\)", re.M)
RE_USING = re.compile(
    r"^Using B1=(\d+), B2=(\d+), polynomial [^,]+, sigma=(\d+):(\d+)", re.M)
RE_FOUND = re.compile(
    r"^Found (prime|composite|probable prime) factor of \d+ digits: (\d+)", re.M)


def parse(text):
    """出力を走査し、(入力式, 因数, b1, b2, param, sigma) を順に返す。

    「Found ...」の直前にある「Using B1=...」を、その因数を見つけた
    カーブのパラメータとみなす。GMP-ECMは1カーブごとにこの順で出力する。
    """
    events = []
    for m in RE_USING.finditer(text):
        events.append(("using", m.start(), m.groups()))
    for m in RE_FOUND.finditer(text):
        events.append(("found", m.start(), m.groups()))
    events.sort(key=lambda e: e[1])

    inputs = [(m.start(), m.group(1)) for m in RE_INPUT.finditer(text)]

    cur = None
    for kind, pos, g in events:
        if kind == "using":
            cur = g
            continue
        if cur is None:
            print(f"  警告: 因数 {g[1]} に対応するカーブ情報が見つかりません",
                  file=sys.stderr)
            continue
        expr = next((e for p, e in reversed(inputs) if p < pos), None)
        b1, b2, param, sigma = cur
        yield expr, g[1], b1, b2, int(param), sigma


def api_id(session, number):
    """数(10進文字列または式)のfactordb IDを引く。"""
    r = session.get(f"{FDB}/api", params={"query": number}, timeout=60)
    r.raise_for_status()
    d = r.json()
    return d.get("id"), d.get("status")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="+", help="GMP-ECMの出力ファイル")
    ap.add_argument("--dry-run", action="store_true",
                    help="送信せず、何を送るかだけ表示する")
    ap.add_argument("--cookie", default=os.environ.get(SESSION_ENV),
                    help=f"fdbuserの値。既定は環境変数 {SESSION_ENV}")
    args = ap.parse_args()

    if not args.cookie and not args.dry_run:
        ap.error(f"--cookie か環境変数 {SESSION_ENV} が必要です")

    session = requests.Session()
    if args.cookie:
        session.cookies.set("fdbuser", args.cookie, domain="factordb.com")

    total = 0
    for path in args.files:
        try:
            text = open(path, encoding="utf-8", errors="replace").read()
        except OSError as e:
            print(f"{path}: 読めません ({e})", file=sys.stderr)
            continue

        for expr, factor, b1, b2, param, sigma in parse(text):
            total += 1
            method = "ecm" if param == 0 else "ecme"
            curve = "Montgomery" if param == 0 else "Edwards"
            print(f"\n■ {path}")
            print(f"  入力     : {expr}")
            print(f"  因数     : {factor} ({len(factor)} 桁)")
            print(f"  B1/B2    : {b1} / {b2}")
            print(f"  sigma    : {param}:{sigma} → {curve} → method={method}")

            if args.dry_run:
                print("  (dry-run のため送信しません)")
                continue

            # 1. 因数を登録する
            r = session.post(f"{FDB}/reportfactor.php",
                             data={"number": expr, "factor": factor}, timeout=120)
            ok = "Factor submitted" in r.text or "already known" in r.text.lower()
            print(f"  因数の登録: HTTP {r.status_code} {'OK' if ok else '要確認'}")
            time.sleep(2)

            # 2. 見つけた方法を登録する(親のIDと因数のIDが要る)
            parent_id, _ = api_id(session, expr)
            time.sleep(1)
            factor_id, _ = api_id(session, factor)
            time.sleep(1)
            if not (parent_id and factor_id):
                print("  方法の登録: IDを引けなかったので中止")
                continue
            r = session.get(f"{FDB}/index.php", timeout=120, params={
                "id": parent_id, "open": "ecm", "cofac": factor_id,
                "method": method, "b1": b1, "b2": b2, "sigma": sigma,
            })
            print(f"  方法の登録: HTTP {r.status_code} "
                  f"(親 {parent_id} / 因数 {factor_id})")
            time.sleep(2)

    if total == 0:
        print("因数は見つかりませんでした。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
