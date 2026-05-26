import numpy as np
from PIL import Image, ImageChops
from skimage.transform import rotate


def trim_white_border(img):
    """画像の外側の白い余白を自動でカットする関数"""
    bg = Image.new(img.mode, img.size, (255, 255, 255))
    diff = ImageChops.difference(img, bg)
    diff = ImageChops.add(diff, diff, 2.0, -100)
    bbox = diff.getbbox()
    if bbox:
        return img.crop(bbox)
    return img


def rotate_and_crop_key_data(image_path, output_path):
    """画像の傾きを自動補正し、中央の鍵データ部分だけを切り出す関数"""
    # 1. 画像を読み込んで余白をカット
    img = Image.open(image_path).convert("RGB")
    img_trimmed = trim_white_border(img)

    # 2. 自動傾き検知（文字の並びの直線性を評価）
    # グレースケール化してnumpy配列に変換
    gray = np.array(img_trimmed.convert("L"))
    # 文字部分（黒）を1、背景（白）を0にするバイナリ化
    thresh = gray < 128

    # -2.0度から2.0度まで0.1度刻みでテストし、最も文字が水平に並ぶ角度（分散が最大になる角度）を探す
    angles = np.arange(-2.0, 2.0, 0.1)
    best_angle = 0
    max_variance = 0

    for angle in angles:
        rotated_thresh = rotate(thresh, angle, order=0, mode="constant", cval=0)
        row_sums = np.sum(rotated_thresh, axis=1)
        variance = np.var(row_sums)
        if variance > max_variance:
            max_variance = variance
            best_angle = angle

    print(f"検知された傾き角度: {best_angle:.2f} 度")

    # 3. 傾きを補正（背景は白で埋める）
    rotated_img_np = (
        rotate(
            np.array(img_trimmed),
            best_angle,
            order=1,
            mode="constant",
            cval=1.0,
        )
        * 255
    ).astype(np.uint8)
    img_rotated = Image.fromarray(rotated_img_np)

    # 4. 補正後の画像から中央データを割合で切り出し
    w, h = img_rotated.size

    # 傾きが直ったため、この比率で上下左右が綺麗に直線を保って切り抜けます
    left_ratio = 0.100  # 左のインデックス「0001:」を削る（微調整用に少し大きめに設定）
    right_ratio = 0.900  # 右のインデックス「:1000」を削る
    top_ratio = 0.033  # 上部タイトルを削る
    bottom_ratio = 0.968  # 下部フッターを削る

    left = int(w * left_ratio)
    top = int(h * top_ratio)
    right = int(w * right_ratio)
    bottom = int(h * bottom_ratio)

    cropped_img = img_rotated.crop((left, top, right, bottom))
    cropped_img.save(output_path)
    print(f"保存完了: {output_path} (サイズ: {cropped_img.size})")


# --- 実行エリア ---
input_file = "/home/teruteru/Documents/page0-1 (コピー)_trim.png"
output_file = "page01_perfect5.png"

try:
    rotate_and_crop_key_data(input_file, output_file)
except Exception as e:
    print(f"エラーが発生しました: {e}")

