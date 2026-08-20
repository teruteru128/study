#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

// CICP（ITU-T H.273）の定義値
#define CICP_PRIMARIES_REC2020  9
#define CICP_TRANSFER_PQ        16
#define CICP_MATRIX_IDENTITY     0
#define CICP_RANGE_FULL         1

int write_hdr_png(const char *filename, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("File open failed");
        return -1;
    }

    // 1. 構造体の初期化
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }

    // エラーハンドリングの設定
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    // 2. 基本的な画像情報の設定（HDRの階調表現のため16bitを選択）
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 16, // HDRを表現するために1チャンネルあたり16bitを使用
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    // 3. cICPチャンク（HDRプロファイル）の設定
    // ※ png_write_info を呼ぶ前に設定する必要があります
    png_set_cICP(png_ptr, info_ptr,
                 CICP_PRIMARIES_REC2020,
                 CICP_TRANSFER_PQ,
                 CICP_MATRIX_IDENTITY,
                 CICP_RANGE_FULL);

    // 4. ヘッダー情報の書き込み
    png_write_info(png_ptr, info_ptr);

    // 16bitデータの場合、Big-Endianで書き込む必要があるため、環境に合わせて自動変換
    png_set_swap(png_ptr);

    // 5. 画像データ（ピクセル）の生成と書き込み
    // 1ピクセルあたり RGB (2バイト * 3 = 6バイト)
    png_bytep row = malloc(width * 6);
    if (!row) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    for (int y = 0; y < height; y++) {
        png_uint_16p ptr = (png_uint_16p)row;
        for (int x = 0; x < width; x++) {
            // テスト用のグラデーション。PQ空間（0.0〜1.0）に応じた輝度値を16bit（0〜65535）で割り当てる
            // 実際の実装では、ここにPQカーブに則ったRGB値を計算して代入します
            uint16_t r = (uint16_t)((x * 65535) / width);
            uint16_t g = (uint16_t)((y * 65535) / height);
            uint16_t b = 32768; // 中間値

            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
        }
        png_write_row(png_ptr, row);
    }

    // 6. 後処理
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

int main() {
    printf("Creating HDR PNG...\n");
    if (write_hdr_png("hdr_output.png", 800, 600) == 0) {
        printf("Success: hdr_output.png created.\n");
    } else {
        printf("Failed to create HDR PNG.\n");
    }
    return 0;
}

