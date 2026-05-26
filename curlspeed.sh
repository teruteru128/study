# 1. 測定用の4.5MBのダミー画像（JPG）を作成
dd if=/dev/urandom of=test_image.jpg bs=1M count=4

# 2. アップロードを実行し、ミリ秒単位で速度を計算
curl -s -o /dev/null \
  -F "file=@test_image.jpg" \
  -w "size_upload:%{size_upload}\ntime_pretransfer:%{time_pretransfer}\ntime_total:%{time_total}\n" \
  https://ascii2d.net | awk -F: '
  /size_upload/ {size=$2}
  /time_pretransfer/ {t_pre=$2}
  /time_total/ {t_tot=$2}
  END {
    duration = t_tot - t_pre;
    if (duration <= 0) duration = 0.000001; # ゼロ除算防止
    # ByteからBitに変換(x8)し、秒あたりのMega bitsに換算
    mbps = (size * 8) / duration / 1000000;
    printf "--- 計測結果 ---\n";
    printf "送信データサイズ: %.2f MB\n", size / 1024 / 1024;
    printf "純粋な転送時間  : %.4f 秒 (%d ミリ秒)\n", duration, duration * 1000;
    printf "実効転送速度    : \033[1;32m%.2f Mbps\033[0m\n", mbps;
  }'

