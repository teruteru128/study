#!/bin/bash

# 監視対象（メンテナンス専用URL）
URL="https://sorry.takarakuji-official.jp/Sorry-PlannedMaintenance.html"

echo "初期状態を取得中..."
INIT_RESPONSE=$(curl -s -I "$URL")
INIT_STATUS=$(echo "$INIT_RESPONSE" | grep -i "^HTTP" | awk '{print $2}' | tail -n1)
INIT_ETAG=$(echo "$INIT_RESPONSE" | grep -i "^etag:" | awk '{print $2}' | tr -d '\r\n')

echo "初期ステータス: $INIT_STATUS"
echo "初期ETag: $INIT_ETAG"
echo "--------------------------------------------------"
echo "【宝くじ公式サイト】メンテ画面の監視を開始します（4時間間隔）..."

# --- [通知用関数] ---
send_notification() {
    local message="$1"
    echo -e "\n$message"
    
    # 警告音を鳴らす
    echo -e "\a\a\a"
    
    # Mac用のデスクトップ通知 (不要なら行頭に#を付けて消してください)
    which osascript > /dev/null && osascript -e "display notification \"宝くじサイトのメンテ画面に変化あり\" with title \"🚨 メンテ監視\" sound name \"Glass\""
    
    # Linux用のデスクトップ通知 (不要なら行頭に#を付けて消してください)
    which notify-send > /dev/null && notify-send "🚨 宝くじサイトメンテ監視" "変化を検知しました"
}

while true; do
    CURRENT_TIME=$(date "+%Y-%m-%d %H:%M:%S")
    
    # HEADメソッドでヘッダー情報を取得
    RESPONSE=$(curl -s -I "$URL")
    STATUS=$(echo "$RESPONSE" | grep -i "^HTTP" | awk '{print $2}' | tail -n1)
    ETAG=$(echo "$RESPONSE" | grep -i "^etag:" | awk '{print $2}' | tr -d '\r\n')

    ALERT_TRIGGERED=false
    REASON=""

    # 判定1: ステータスが「200 OK」から別のコード（404や302など）に変わった場合
    # ＝ メンテナンス画面自体が撤去された、またはリダイレクトされた可能性
    if [ "$INIT_STATUS" = "200" ] && [ "$STATUS" != "200" ] && [ -n "$STATUS" ]; then
        ALERT_TRIGGERED=true
        REASON="メンテ画面が消失、または移動しました！ (Status: $INIT_STATUS -> $STATUS)"
    fi

    # 判定2: ETagが初期状態から変化した場合
    # ＝ メンテ画面内のお知らせやテキストが更新された可能性
    if [ "$ETAG" != "$INIT_ETAG" ]; then
        ALERT_TRIGGERED=true
        REASON="メンテ画面のETagが変更されました！お知らせが更新された可能性があります。"
    fi

    # 変化を検知した場合
    if [ "$ALERT_TRIGGERED" = true ]; then
        MSG="[$CURRENT_TIME] 🚨 宝くじメンテ画面に変化を検知！\n理由: $REASON\n確認先: $URL"
        send_notification "$MSG"
        
        # 基準状態を最新に更新して監視を継続
        INIT_STATUS="$STATUS"
        INIT_ETAG="$ETAG"
    else
        echo "[$CURRENT_TIME] 監視中... 変化なし (Status: $STATUS, ETag: ${ETAG:-未発行})"
    fi

    # 4時間（14400秒）待機
    sleep 14400
done

