#!/bin/bash
# 再起動後の確認(読み取り専用)。何も再起動せず、設定も変えない。
#
# 使い方: ./after-reboot-check.sh [他マシンのssh接続先...]
#   他マシンは引数、または環境変数AFTER_REBOOT_HOSTS(空白区切り)で渡す。
#   ホスト名やアドレスはCLAUDE.local.mdを参照(このリポジトリには書かない)。
#
# 確認すること:
#   1. 稼働時間(再起動があったか)
#   2. Postgresが、postgresql.confのlisten_addressesに書いたアドレスすべてで待ち受けているか
#      (2026-10-02に、起動がネットワークより早くてLAN側・Tailscale側にバインドできず、
#       他マシンの素数探索が約7時間止まった。その再発の検出)
#   3. 素数探索のサービスの状態と、起動ログの「mr2fs高速経路: 有効」
#   4. 他マシンの素数探索と、そこからDBへの到達
#   5. ECMが走っていないこと(2026-10-03から、素数探索2本が見つかるまで凍結する方針。表示のみ)
#
# 終了コード: NGの数(0なら問題なし)
set -u

ng=0
ok()   { echo "  OK   $*"; }
ngl()  { echo "  NG   $*"; ng=$((ng + 1)); }
info() { echo "  INFO $*"; }

# 素数探索のサービス確認。ローカルでも、ssh先でも動くように関数1つにまとめてある
# (ssh先では declare -f で関数ごと送る)。マスクされたunitは対象外。
check_prime_search() {
  local units u state since log
  units=$(systemctl --user list-units 'prime-search@*.service' --all --no-legend --plain 2>/dev/null \
    | awk '{print $1}')
  if [ -z "$units" ]; then
    echo "  INFO prime-search@*.service が見つからない(このマシンでは動かしていない?)"
    return 0
  fi
  for u in $units; do
    state=$(systemctl --user is-active "$u" 2>/dev/null)
    if [ "$state" != "active" ]; then
      echo "  NG   $u: $state"
      continue
    fi
    since=$(systemctl --user show "$u" -p ActiveEnterTimestamp --value)
    log=$(journalctl --user -u "$u" --since "$since" --no-pager 2>/dev/null | grep "mr2fs高速経路" | tail -1)
    case "$log" in
      *有効*) echo "  OK   $u: 稼働中、mr2fs高速経路 有効 (開始 $since)" ;;
      *無効*) echo "  NG   $u: 稼働中だがmr2fs高速経路が無効 (GMPのみで約2倍遅い): ${log##*: }" ;;
      *)      echo "  NG   $u: 稼働中だがmr2fsの有効/無効がログに見つからない (開始 $since)" ;;
    esac
  done
  # DBへの到達(db.envのDB_URLのホスト:ポートだけ取り出す。パスワードは見ない)
  local conf="$HOME/.config/prime-search/db.env" hp host port
  if [ -r "$conf" ]; then
    hp=$(sed -n 's#.*jdbc:postgresql://\([^/?"]*\).*#\1#p' "$conf" | head -1)
    host=${hp%%:*}
    port=${hp##*:}
    [ "$port" = "$hp" ] && port=5432
    if [ -n "$host" ] && timeout 5 bash -c "exec 3<>/dev/tcp/$host/$port" 2>/dev/null; then
      echo "  OK   DB ($host:$port) に到達できる"
    else
      echo "  NG   DB ($host:$port) に到達できない"
    fi
  else
    echo "  INFO $conf が読めない(DB到達は確認しない)"
  fi
}

echo "== 1. 稼働時間 =="
echo "  起動: $(uptime -s)  ($(uptime -p))"
up=$(cut -d. -f1 /proc/uptime)
if [ "$up" -lt 86400 ]; then
  info "24時間以内に再起動している。以降の確認は特に重要"
fi

echo "== 2. Postgresの待ち受け =="
conf=$(ls /etc/postgresql/*/main/postgresql.conf 2>/dev/null | tail -1)
if [ -z "$conf" ] || ! [ -r "$conf" ]; then
  info "postgresql.confが読めない/無い(このマシンはDBサーバーではない?)"
else
  listen=$(sed -n "s/^[[:space:]]*listen_addresses[[:space:]]*=[[:space:]]*'\([^']*\)'.*/\1/p" "$conf" | tail -1)
  port=$(sed -n 's/^[[:space:]]*port[[:space:]]*=[[:space:]]*\([0-9]*\).*/\1/p' "$conf" | tail -1)
  port=${port:-5432}
  listen=${listen:-localhost}
  actual=$(ss -ltnH "sport = :$port" 2>/dev/null | awk '{print $4}' | sed "s/:$port\$//")
  info "listen_addresses='$listen' port=$port"
  IFS=',' read -ra want <<< "$listen"
  for a in "${want[@]}"; do
    a=$(echo "$a" | tr -d ' ')
    case "$a" in
      localhost) a=127.0.0.1 ;;
      '*')       a=0.0.0.0 ;;
    esac
    if echo "$actual" | grep -qxF "$a"; then
      ok "$a:$port で待ち受けている"
    else
      ngl "$a:$port で待ち受けていない → ネットワーク起動前にPostgresが上がった可能性。sudo systemctl restart postgresql@<version>-main"
    fi
  done
fi

echo "== 3. このマシンの素数探索 =="
out=$(check_prime_search)
echo "$out"
ng=$((ng + $(echo "$out" | grep -c '^  NG')))

echo "== 4. 他のマシン =="
hosts=("$@")
if [ ${#hosts[@]} -eq 0 ] && [ -n "${AFTER_REBOOT_HOSTS:-}" ]; then
  read -ra hosts <<< "$AFTER_REBOOT_HOSTS"
fi
if [ ${#hosts[@]} -eq 0 ]; then
  info "他のマシンは指定されていない(引数かAFTER_REBOOT_HOSTSで渡す)"
fi
for h in "${hosts[@]}"; do
  echo " -- $h"
  out=$(ssh -o BatchMode=yes -o VisualHostKey=no -o ConnectTimeout=10 "$h" \
    "$(declare -f check_prime_search); check_prime_search" 2>/dev/null)
  if [ -z "$out" ]; then
    ngl "$h にsshで接続できない、または出力が無い"
    continue
  fi
  echo "$out"
  ng=$((ng + $(echo "$out" | grep -c '^  NG')))
done

echo "== 5. ECM(凍結中の方針) =="
if pgrep -x ecm > /dev/null; then
  info "ecmが走っている($(pgrep -xc ecm)プロセス)。素数探索2本が見つかるまで凍結する方針なので、意図したものか確認"
else
  ok "ecmは走っていない"
fi

echo
if [ "$ng" -eq 0 ]; then
  echo "結果: 問題なし"
else
  echo "結果: NGが ${ng} 件ある"
fi
exit "$ng"
