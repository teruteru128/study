
#include "pngheaders.h"
#include <complex.h>
#include <png.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 2560
#define HEIGHT 1920
#define MAX_ITER 1000

// 数学的に正しい判定関数
int mandelbrot(float complex c) {
    float complex z = 0;
    for (int i = 0; i < MAX_ITER; i++) {
        // |z|^2 > 4 ならば発散とみなす (cabsfより高速)
        if (crealf(z)*crealf(z) + cimagf(z)*cimagf(z) > 4.0f) {
            return 0; // 集合の外
        }
        z = z * z + c;
    }
    return 1; // 集合の内
}

int main() {
    struct IHDR ihdr = {
        .width = WIDTH, .height = HEIGHT, .bit_depth = 8,
        .color_type = PNG_COLOR_TYPE_RGB, .interlace_method = PNG_INTERLACE_NONE,
        .compression_method = PNG_COMPRESSION_TYPE_DEFAULT, .filter_method = PNG_NO_FILTERS
    };

    png_byte **data = malloc(HEIGHT * sizeof(png_byte *));
    for (size_t y = 0; y < HEIGHT; y++) {
        data[y] = malloc(3 * WIDTH * sizeof(png_byte));
    }

    const float complex left_top = -2.0f + 1.0f*I;
    const float unit = 2.0f / HEIGHT; // 縦の範囲を基準にスケーリング

    // 行優先ループ (キャッシュ効率のため y が外側)
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            float complex c = left_top + (float)x * unit - (float)y * unit * I;
            
            int is_inside = mandelbrot(c);
            unsigned char color = is_inside ? 0x00 : 0xFF; // 内側は黒、外側は白

            data[y][x*3 + 0] = color;
            data[y][x*3 + 1] = color;
            data[y][x*3 + 2] = color;
        }
    }

    if (write_png("out.png", &ihdr, NULL, NULL, 0, data) == 0) {
        printf("Success: out.png generated.\n");
    } else {
        fprintf(stderr, "Error: Failed to write PNG.\n");
    }

    // メモリ解放
    for (size_t y = 0; y < HEIGHT; y++) free(data[y]);
    free(data);

    return 0;
}
