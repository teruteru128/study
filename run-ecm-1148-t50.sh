#!/bin/bash
# 2^1148*21181+1 (350桁) を t50 水準まで ECM で掘る。
#   B1=43,000,000 / GMP-ECM推奨 17,769 カーブ
# 既存の 1148_ecm_50digits.txt に追記し、カーブ数はそこから数えるので
# 中断しても続きから再開できる。因数が見つかったら即座に止まる。
set -u
cd /home/teruteru/Documents/Projects/teruteru128/study
FILE=1148_ecm_50digits.txt
TARGET=17769
B1=43000000
JOBS=${JOBS:-4}          # 同時プロセス数。素数探索と分け合うため既定は控えめ
BATCH=${BATCH:-5}        # 1プロセスあたりのカーブ数
N="2^1148*21181+1"

count() { grep -ac "^Using B1=" "$FILE" 2>/dev/null || echo 0; }
found() { grep -aqiE "^Found (prime|composite|probable)" "$FILE" 2>/dev/null; }

for pid in "$@"; do
  while kill -0 "$pid" 2>/dev/null; do sleep 30; done
done

echo "=== ECM 開始: $(date -Is)  ${JOBS}プロセス × ${BATCH}カーブ ==="
echo "    開始時点: $(count)/$TARGET カーブ"
while [ "$(count)" -lt "$TARGET" ]; do
  if found; then echo "★ 因数が見つかりました: $(date -Is)"; break; fi
  tmpd=$(mktemp -d)
  for i in $(seq "$JOBS"); do
    echo "$N" | ecm -maxmem 400 -c "$BATCH" "$B1" > "$tmpd/$i.out" 2>&1 &
  done
  wait
  cat "$tmpd"/*.out >> "$FILE"      # 追記はまとめて行い、出力の混線を防ぐ
  rm -rf "$tmpd"
  echo "$(date +%H:%M:%S)  $(count)/$TARGET カーブ"
done
echo "=== ECM 終了: $(date -Is)  $(count)/$TARGET カーブ ==="
found && grep -aiE "^Found" "$FILE" | tail -3
