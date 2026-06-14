#!/usr/bin/env bash
echo "接続開始: $(date '+%Y-%m-%d %H:%M:%S')"
while true; do
  # curlでTorプロキシを経由し、HEADリクエスト(-I)を送信
  # 接続成功（終了ステータス0）なら時刻を表示してループを抜ける
  if curl --socks5-hostname 127.0.0.1:9050 -I -s --max-time 30 "http://apow7mjfryruh65chtdydfmqfpj5btws7nbocgtaovhvezgccyjazpqd.onion/torproject.org/dists/noble/InRelease" > /dev/null; then
    echo "接続成功: $(date '+%Y-%m-%d %H:%M:%S')"
    break
  fi
  
  # 5分（300秒）待機
  sleep 300
done

