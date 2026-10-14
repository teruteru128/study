#!/bin/bash
# esieve の出力 (id p step の行) を candidates テーブルに反映する。
#
#   ./esieve-apply.sh hits.tsv [hits2.tsv ...]            ドライラン (件数を数えるだけ。何も変更しない)
#   ./esieve-apply.sh --apply hits.tsv [hits2.tsv ...]    反映する
#
# 反映の中身: 未判定 (composite=0 and probably_prime=0 and definitely_prime=0) で、まだ誰も
# 着手していない (timeassigned is null) 行を composite=2, resultclient='esieve' にする。
#  - composite=2 は「篩で合成数と判明」。PrimeSearchTask2.claim() は composite=0 を要求するので、
#    稼働中のワーカーも着手の瞬間に自然にスキップする(再起動は不要)。
#  - composite=1 (MRで合成数) とは区別する。backup-primesearch.sh の「判定済み件数」が汚れない。
#  - 着手済みの行は触らない。ワーカーが後から composite+1 を書くので、ここで更新するとぶつかる。
# 何度流しても同じ結果になる (冪等)。
set -euo pipefail
cd "$(dirname "$0")"

APPLY=0
if [ "${1:-}" = "--apply" ]; then APPLY=1; shift; fi
if [ $# -lt 1 ]; then
  sed -n '2,15p' "$0" | sed 's/^# \{0,1\}//'
  exit 1
fi
for f in "$@"; do [ -r "$f" ] || { echo "読めません: $f" >&2; exit 1; }; done

{
  echo "begin;"
  echo "create temp table esieve_hits(id bigint, p bigint, step integer) on commit drop;"
  echo "copy esieve_hits from stdin (delimiter ' ');"
  cat "$@"
  echo '\.'
  echo "create temp table esieve_uniq on commit drop as select distinct id, step from esieve_hits;"
  echo "select count(*) as 当たり行数, (select count(*) from esieve_uniq) as 重複除去後 from esieve_hits;"
  echo "select u.id, count(*) as 当たり,
          count(*) filter (where c.step is not null) as 候補表にある,
          count(*) filter (where c.composite = 0 and c.probably_prime = 0 and c.definitely_prime = 0
                                 and c.timeassigned is null) as 反映対象,
          count(*) filter (where c.composite = 0 and c.probably_prime = 0 and c.definitely_prime = 0
                                 and c.timeassigned is not null) as 着手済みで除外,
          count(*) filter (where c.composite = 2) as 反映済み
        from esieve_uniq u left join candidates c using (id, step) group by u.id order by u.id;"
  if [ "$APPLY" = 1 ]; then
    echo "update candidates c set composite = 2, resultclient = 'esieve'
          from esieve_uniq u
          where c.id = u.id and c.step = u.step and c.composite = 0 and c.probably_prime = 0
            and c.definitely_prime = 0 and c.timeassigned is null;"
    echo "commit;"
  else
    echo "rollback;"
  fi
} | ./psql-primesearch.sh -v ON_ERROR_STOP=1 -X
if [ "$APPLY" = 1 ]; then echo "反映しました"; else echo "(ドライラン: 何も変更していません。反映するには --apply)"; fi
