#!/bin/bash

# 対象URL
URL="https://sorry.takarakuji-official.jp/Sorry-PlannedMaintenance.html"

# 初回の状態を自動で取得して基準にする（手動セット不要）
echo "初期状態を取得中..."
INIT_RESPONSE=$(curl -s -I "$URL")
INIT_STATUS=$(echo "$INIT_RESPONSE" | grep -i "^HTTP" | awk '{print $2}' | tail -n1)
INIT_ETAG=$(echo "$INIT_RESPONSE" | grep -i "^etag:" | awk '{print $2}' | tr -d '\r\n')

echo "初期ステータス: $INIT_STATUS"
echo "初期ETag: $INIT_ETAG"
echo "--------------------------------------------------"
echo "【宝くじ公式サイト】復旧・更新の監視を開始します（5分間隔）..."

# --- [通知用関数] お好みの通知方法のコメントアウトを解除してください ---
send_notification() {
    local message="$1"
    echo -e "\n$message"
    
    # 【方法A】デスクトップ通知 (Macの場合)
    which osascript > /dev/null && osascript -e "display notification \"$message\" with title \"宝くじサイト監視\""
    
    # 【方法B】デスクトップ通知 (Linux/Ubuntuの場合)
    # which notify-send > /dev/null && notify-send "宝くじサイト監視" "$message"
    
    # 【方法C】LINE Notifyでスマホに通知 (アクセストークンがある場合)
    # LINE_TOKEN="ここにあなたのLINEトークンを入れる"
    # curl -s -X POST -H "Authorization: Bearer $LINE_TOKEN" -F "message=$message" https://line.me > /dev/null
}

while true; do
    CURRENT_TIME=$(date "+%Y-%m-%d %H:%M:%S")
    
    # HEADメソッドでヘッダー情報を取得
    RESPONSE=$(curl -s -I "$URL")
    STATUS=$(echo "$RESPONSE" | grep -i "^HTTP" | awk '{print $2}' | tail -n1)
    ETAG=$(echo "$RESPONSE" | grep -i "^etag:" | awk '{print $2}' | tr -d '\r\n')

    # 判定フラグ
    ALERT_TRIGGERED=false
    REASON=""

    # 条件1: ステータスが200以外（503や302など）から「200 OK」に変わった場合
    if [ "$INIT_STATUS" != "200" ] && [ "$STATUS" = "200" ]; then
        ALERT_TRIGGERED=true
        REASON="サイトが200 OK（アクセス可能）になりました！"
    fi

    # 条件2: ETagが初期状態から変化した場合（お知らせ等の更新）
    # ※ETagが空から値が入った場合、または値が変わった場合
    if [ "$ETAG" != "$INIT_ETAG" ]; then
        ALERT_TRIGGERED=true
        REASON="ETagが変更されました！お知らせやコンテンツが更新された可能性があります。 (旧: $INIT_ETAG -> 新: $ETAG)"
    fi

    # トリガーを検知した場合
    if [ "$ALERT_TRIGGERED" = true ]; then
        MSG="[$CURRENT_TIME] 🚨 宝くじ公式サイトに変化を検知！\n理由: $REASON\nURL: $URL"
        send_notification "$MSG"
        
        # 連続で通知が飛び続けないよう、基準を最新の状態に更新して監視を継続
        INIT_STATUS="$STATUS"
        INIT_ETAG="$ETAG"
        echo "基準状態を最新（Status: $STATUS, ETag: $ETAG）に更新しました。"
    else
        echo "[$CURRENT_TIME] 監視中... 変化なし (Status: $STATUS, ETag: ${ETAG:-未発行})"
    fi

    # サーバー負荷防止（5分待機）
    sleep 300
done

