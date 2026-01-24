
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct {
    int w, h, n;
    unsigned char *data;
} Image;

int main(int argc, char **argv) {
    // 引数チェック: モード、出力名、入力ファイル(最低2枚以上)
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <v|h> <output> <in1> <in2> <in3> ...\n", argv[0]);
        return 1;
    }

    char mode = argv[1][0];
    const char *out_name = argv[2];
    int num_inputs = argc - 3;

    // 画像情報を格納する構造体配列を確保
    Image *imgs = (Image *)malloc(sizeof(Image) * num_inputs);
    if (!imgs) return 1;

    int total_w = 0;
    int total_h = 0;

    // 全画像を読み込み、出力サイズを計算
    for (int i = 0; i < num_inputs; i++) {
        imgs[i].data = stbi_load(argv[i + 3], &imgs[i].w, &imgs[i].h, &imgs[i].n, 4);
        if (!imgs[i].data) {
            fprintf(stderr, "Error: Could not load %s\n", argv[i + 3]);
            return 1;
        }

        if (mode == 'h') {
            total_w += imgs[i].w;
            if (imgs[i].h > total_h) total_h = imgs[i].h; // 最大の高さに合わせる
        } else {
            total_h += imgs[i].h;
            if (imgs[i].w > total_w) total_w = imgs[i].w; // 最大の幅に合わせる
        }
    }

    // 出力用バッファ確保 (RGBA 4ch)
    unsigned char *out_data = (unsigned char *)calloc(total_w * total_h * 4, 1);
    if (!out_data) return 1;

    // 各画像をキャンバスにコピー
    int current_offset_x = 0;
    int current_offset_y = 0;

    for (int i = 0; i < num_inputs; i++) {
        for (int y = 0; y < imgs[i].h; y++) {
            unsigned char *src_row = &imgs[i].data[y * imgs[i].w * 4];
            unsigned char *dst_row = &out_data[((y + current_offset_y) * total_w + current_offset_x) * 4];
            memcpy(dst_row, src_row, imgs[i].w * 4);
        }

        // 次の画像の配置オフセットを更新
        if (mode == 'h') {
            current_offset_x += imgs[i].w;
        } else {
            current_offset_y += imgs[i].h;
        }
    }

    // 書き出し
    if (stbi_write_png(out_name, total_w, total_h, 4, out_data, total_w * 4)) {
        printf("Success: %s saved (%dx%d) from %d images.\n", out_name, total_w, total_h, num_inputs);
    } else {
        fprintf(stderr, "Error: Failed to save image.\n");
    }

    // 後片付け
    for (int i = 0; i < num_inputs; i++) {
        stbi_image_free(imgs[i].data);
    }
    free(imgs);
    free(out_data);

    return 0;
}

