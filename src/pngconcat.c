
#include "pngheaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 画像1データを指定した位置にコピーする汎用関数
void copy_image_region(png_byte **dst, png_byte **src, 
                       size_t src_width, size_t src_height, 
                       size_t offset_x) {
    for (size_t y = 0; y < src_height; y++) {
        // RGBA (4 bytes per pixel) を前提
        memcpy(&dst[y][offset_x * 4], src[y], src_width * 4);
    }
}

// 画像データのメモリ解放
void free_image_data(png_byte **data, size_t height) {
    if (data == NULL) return;
    for (size_t y = 0; y < height; y++) {
        free(data[y]);
    }
    free(data);
}

// 指定したサイズのキャンバスを確保
png_byte **allocate_image_data(size_t width, size_t height) {
    png_byte **data = malloc(sizeof(png_byte *) * height);
    if (!data) return NULL;
    for (size_t y = 0; y < height; y++) {
        data[y] = malloc(sizeof(png_byte) * 4 * width);
        if (!data[y]) return NULL; // 本来は確保済みの分も解放すべき
    }
    return data;
}

int main(int argc, char const *argv[]) {
    const char *base_dir = "/mnt/g/iandm/image/shonenjumpplus.com/13933686331749163174/";
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];

    // ループ範囲やファイル名のルールを汎用化
    for (size_t i = 1; i < 60; i += 2) {
        struct IHDR ihdr[2];
        png_byte **input_data[2] = {NULL, NULL};

        // 2つの画像を読み込み
        for (int j = 0; j < 2; j++) {
            snprintf(input_path, PATH_MAX, "%spage%02zu.png", base_dir, i + j);
            if (read_png(input_path, &ihdr[j], NULL, NULL, NULL, &input_data[j]) != 0) {
                fprintf(stderr, "Failed to read: %s\n", input_path);
                continue; 
            }
        }

        if (!input_data[0] || !input_data[1]) continue;

        // 出力サイズの設定（1枚目のサイズを基準に横に並べる）
        size_t single_width = ihdr[0].width;
        size_t out_height = ihdr[0].height;
        size_t out_width = single_width * 2;

        struct IHDR out_ihdr = ihdr[0]; // 基本設定をコピー
        out_ihdr.width = out_width;
        out_ihdr.color_type = PNG_COLOR_TYPE_RGBA;

        // 出力用キャンバス確保
        png_byte **output_data = allocate_image_data(out_width, out_height);
        if (output_data) {
            // 右側に1枚目、左側に2枚目を結合（元のコードの挙動を維持）
            copy_image_region(output_data, input_data[0], single_width, out_height, single_width);
            copy_image_region(output_data, input_data[1], single_width, out_height, 0);

            // 書き出し
            snprintf(output_path, PATH_MAX, "%swork/page%02zu-%02zu.png", base_dir, i, i + 1);
            write_png(output_path, &out_ihdr, NULL, NULL, 0, output_data);

            // キャンバス解放
            free_image_data(output_data, out_height);
        }

        // 入力データ解放
        free_image_data(input_data[0], ihdr[0].height);
        free_image_data(input_data[1], ihdr[1].height);
        
        printf("Saved: %s\n", output_path);
    }

    return 0;
}

