
#include "pngheaders.h"
#include <complex.h>
#include <png.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 3840
#define HEIGHT 2160
#define MAX_ITER 10000
#define THREAD_COUNT 16  // 使用するスレッド数

typedef struct {
    png_byte **data;
    long double center_re;
    long double center_im;
    long double range;
} SharedData;

typedef struct { int start_y, end_y; SharedData *shared; } ThreadArg;

// 色付け：対数スケーリング（深いズームで色を滑らかにする手法）
void get_color(int iter, unsigned char *rgb) {
    if (iter == MAX_ITER) {
        rgb[0] = rgb[1] = rgb[2] = 0;
        return;
    }
    // 周期的な色変化
    rgb[0] = (unsigned char)(127.5 * (1.0 + sin(0.1 * iter)));
    rgb[1] = (unsigned char)(127.5 * (1.0 + sin(0.1 * iter + 2.0)));
    rgb[2] = (unsigned char)(127.5 * (1.0 + sin(0.1 * iter + 4.0)));
}

void *thread_func(void *arg) {
    ThreadArg *t_arg = (ThreadArg *)arg;
    SharedData *s = t_arg->shared;
    long double unit = s->range / HEIGHT;

    for (int y = t_arg->start_y; y < t_arg->end_y; y++) {
        long double c_imag = s->center_im + (HEIGHT / 2.0L - y) * unit;
        for (int x = 0; x < WIDTH; x++) {
            long double c_real = s->center_re + (x - WIDTH / 2.0L) * unit;
            
            long double z_re = 0, z_im = 0;
            int iter = 0;
            // 数学的な最適化：z^2 の計算をインライン化
            while (z_re*z_re + z_im*z_im <= 4.0L && iter < MAX_ITER) {
                long double next_re = z_re*z_re - z_im*z_im + c_real;
                z_im = 2.0L * z_re * z_im + c_imag;
                z_re = next_re;
                iter++;
            }
            get_color(iter, &s->data[y][x * 3]);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char *filename = "deep_zoom.png";
    // 例：Seahorse Valley（タツノオトシゴの谷）の深い座標
    long double cx = -0.74364388703715870833L;
    long double cy = 0.13182590420641924031L;
    long double range = 0.00000000000001L; // 非常に深いズーム

    if (argc >= 5) {
        filename = argv[1];
        cx = strtold(argv[2], NULL); // long double 用の変換
        cy = strtold(argv[3], NULL);
        range = strtold(argv[4], NULL);
    }

    png_byte **data = malloc(HEIGHT * sizeof(png_byte *));
    for (size_t y = 0; y < HEIGHT; y++) data[y] = malloc(3 * WIDTH);

    SharedData shared = { .data = data, .center_re = cx, .center_im = cy, .range = range };
    pthread_t threads[THREAD_COUNT];
    ThreadArg args[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        args[i] = (ThreadArg){ .start_y = i * (HEIGHT/THREAD_COUNT), .end_y = (i+1) * (HEIGHT/THREAD_COUNT), .shared = &shared };
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }
    for (int i = 0; i < THREAD_COUNT; i++) pthread_join(threads[i], NULL);

    // 座標計算と表示
    long double unit = range / HEIGHT;
    long double left_top_re = cx - (WIDTH / 2.0L) * unit;
    long double left_top_im = cy + (HEIGHT / 2.0L) * unit;
    long double right_bottom_re = cx + (WIDTH / 2.0L) * unit;
    long double right_bottom_im = cy - (HEIGHT / 2.0L) * unit;

    printf("\n--- Rendering Info ---\n");
    printf("File:   %s\n", filename);
    printf("Center: (%.20Lf, %.20Lf)\n", cx, cy);
    printf("Top-Left:     (%.20Lf, %.20Lf)\n", left_top_re, left_top_im);
    printf("Bottom-Right: (%.20Lf, %.20Lf)\n", right_bottom_re, right_bottom_im);
    printf("----------------------\n");

    struct IHDR ihdr = { .width = WIDTH, .height = HEIGHT, .bit_depth = 8, .color_type = PNG_COLOR_TYPE_RGB };
    write_png(filename, &ihdr, NULL, NULL, 0, data);

    for (size_t y = 0; y < HEIGHT; y++) free(data[y]);
    free(data);
    printf("Saved to %s (Precision: long double)\n", filename);
    return 0;
}
