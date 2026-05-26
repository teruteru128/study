#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

// PNG画像を読み込む関数
int read_png(const char *filename, int *width, int *height, png_bytep **row_pointers, png_byte *color_type, png_byte *bit_depth) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return 0; }

    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); fclose(fp); return 0; }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return 0;
    }

    png_init_io(png, fp);
    png_read_info(png, info);

    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    *color_type = png_get_color_type(png, info);
    *bit_depth = png_get_bit_depth(png, info);

    // RGBまたはRGBA形式に変換を強制（パレットやグレースケールを統一するため）
    if (*color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (*color_type == PNG_COLOR_TYPE_GRAY && *bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (*bit_depth == 16) png_set_strip_16(png);
    if (*color_type == PNG_COLOR_TYPE_GRAY || *color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);

    png_read_update_info(png, info);
    
    // 更新後のステータスを取得
    *color_type = png_get_color_type(png, info);
    size_t row_bytes = png_get_rowbytes(png, info);

    *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
    for (int y = 0; y < *height; y++) {
        (*row_pointers)[y] = (png_byte*)malloc(row_bytes);
    }

    png_read_image(png, *row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return 1;
}

// PNG画像を書き出す関数
int write_png(const char *filename, int width, int height, png_bytep *row_pointers, png_byte color_type, png_byte bit_depth) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return 0;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return 0; }

    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_write_struct(&png, NULL); fclose(fp); return 0; }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return 0;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return 1;
}

// メモリ解放
void free_rows(png_bytep *row_pointers, int height) {
    if (!row_pointers) return;
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input1.png> <input2.png> <output.png>\n", argv[0]);
        return 1;
    }

    int w1, h1, w2, h2;
    png_byte color1, depth1, color2, depth2;
    png_bytep *rows1 = NULL, *rows2 = NULL, *rows_diff = NULL;

    // 2枚の画像を読み込み
    if (!read_png(argv[1], &w1, &h1, &rows1, &color1, &depth1) ||
        !read_png(argv[2], &w2, &h2, &rows2, &color2, &depth2)) {
        fprintf(stderr, "Error reading input files.\n");
        return 1;
    }

    // 解像度の一致を確認
    if (w1 != w2 || h1 != h2) {
        fprintf(stderr, "Error: Image dimensions do not match. (%dx%d) vs (%dx%d)\n", w1, h1, w2, h2);
        free_rows(rows1, h1);
        free_rows(rows2, h2);
        return 1;
    }

    // 差分画像のメモリを確保
    rows_diff = (png_bytep*)malloc(sizeof(png_bytep) * h1);
    int channels = (color1 == PNG_COLOR_TYPE_RGBA) ? 4 : 3; // 読み込み時RGB(A)化しているため
    size_t row_bytes = w1 * channels;
    for (int y = 0; y < h1; y++) {
        rows_diff[y] = (png_byte*)malloc(row_bytes);
    }

    // ピクセルごとに差分（絶対値）を計算
    for (int y = 0; y < h1; y++) {
        png_bytep row1 = rows1[y];
        png_bytep row2 = rows2[y];
        png_bytep rowd = rows_diff[y];

        for (int x = 0; x < w1 * channels; x += channels) {
            // R, G, Bの差分
            rowd[x]     = abs(row1[x]     - row2[x]);     // Red
            rowd[x + 1] = abs(row1[x + 1] - row2[x + 1]); // Green
            rowd[x + 2] = abs(row1[x + 2] - row2[x + 2]); // Blue
            
            // アルファチャンネル（透過）の処理
            if (channels == 4) {
                rowd[x + 3] = 255; // 差分画像は見やすいように不透明にする
            }
        }
    }

    // 差分画像の保存
    if (!write_png(argv[3], w1, h1, rows_diff, color1, depth1)) {
        fprintf(stderr, "Error writing output file.\n");
    } else {
        printf("Success: Diff image saved to %s\n", argv[3]);
    }

    // メモリ解放
    free_rows(rows1, h1);
    free_rows(rows2, h2);
    free_rows(rows_diff, h1);

    return 0;
}

