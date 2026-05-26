import os
from PIL import Image, ImageChops


def trim_white_border(img):
    """画像の外側の白い余白を自動でカットする関数"""
    # 背景が白（255）に近い部分を検出するためのマスクを作成
    bg = Image.new(img.mode, img.size, (255, 255, 255))
    diff = ImageChops.difference(img, bg)
    diff = ImageChops.add(diff, diff, 2.0, -100)
    bbox = diff.getbbox()
    if bbox:
        return img.crop(bbox)
    return img


def crop_key_data(image_path, output_path):
    """中央の鍵データ部分だけを割合で切り出す関数"""
    img = Image.open(image_path).convert("RGB")

    # 1. まず外側の余計な白の余白をカットして基準を揃える
    img_trimmed = trim_white_border(img)
    w, h = img_trimmed.size

    # 2. 余白カット後の画像に対する中央データの割合（％）を指定
    # 左右の「0001:」や「:1000」などのインデックスを完全に除外する範囲
    left_ratio = 0.61  # 左端から12%の位置からスタート
    right_ratio = 0.881  # 右端から88%の位置まで（右側12%をカット）
    top_ratio = 0.11  # 上部タイトル「The modulus of...」の下から
    bottom_ratio = 0.88  # 下部「1001-2000」などの上まで

    # 座標の計算
    left = int(w * left_ratio)
    top = int(h * top_ratio)
    right = int(w * right_ratio)
    bottom = int(h * bottom_ratio)

    # 3. 切り出しを実行
    cropped_img = img_trimmed.crop((left, top, right, bottom))

    # 保存
    cropped_img.save(output_path)
    print(f"Saved: {output_path} (Size: {cropped_img.size})")


# --- 実行エリア ---
# 画像ファイル名（適宜書き換えてください）
input_file = "/home/teruteru/Documents/page0-1 (コピー).png"
output_file = "page01_cropped.png"

if os.path.exists(input_file):
    crop_key_data(input_file, output_file)
else:
    print(f"エラー: {input_file} が見つかりません。")

