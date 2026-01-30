
#include "pngheaders.h"
#include <complex.h>
#include <immintrin.h>
#include <png.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 3840
#define HEIGHT 2160
#define MAX_ITER 512
#define THREAD_COUNT 16  // 使用するスレッド数

typedef struct {
    png_byte **data;
    double center_re; // ズーム中心(実部)
    double center_im; // ズーム中心(虚部)
    double range;     // 表示範囲（小さいほどズーム）
} SharedData;

typedef struct { int start_y, end_y; SharedData *shared; } ThreadArg;

// 色付け：反復回数に基づいて青系のグラデーション
void get_color(int iter, unsigned char *rgb) {
    if (iter == MAX_ITER) {
        rgb[0] = rgb[1] = rgb[2] = 0;
        return;
    }
    rgb[0] = (iter * 2) % 255;
    rgb[1] = (iter * 5) % 255;
    rgb[2] = (iter * 13) % 255;
}

void *thread_func(void *arg) {
    ThreadArg *t_arg = (ThreadArg *)arg;
    SharedData *s = t_arg->shared;
    double aspect_ratio = (double)WIDTH / HEIGHT;
    double unit = s->range / HEIGHT;

    for (int y = t_arg->start_y; y < t_arg->end_y; y++) {
        float c_imag = (float)(s->center_im + (HEIGHT / 2.0 - y) * unit);
        __m256 c_imag_v = _mm256_set1_ps(c_imag);

        for (int x = 0; x < WIDTH; x += 8) {
            float start_re = (float)(s->center_re + (x - WIDTH / 2.0) * unit);
            __m256 c_real_v = _mm256_set_ps(
                start_re + 7*(float)unit, start_re + 6*(float)unit,
                start_re + 5*(float)unit, start_re + 4*(float)unit,
                start_re + 3*(float)unit, start_re + 2*(float)unit,
                start_re + 1*(float)unit, start_re + 0*(float)unit
            );

            __m256 z_re = _mm256_setzero_ps();
            __m256 z_im = _mm256_setzero_ps();
            __m256i iterations = _mm256_setzero_si256();
            __m256 four = _mm256_set1_ps(4.0f);

            for (int i = 0; i < MAX_ITER; i++) {
                __m256 re2 = _mm256_mul_ps(z_re, z_re);
                __m256 im2 = _mm256_mul_ps(z_im, z_im);
                __m256 mask = _mm256_cmp_ps(_mm256_add_ps(re2, im2), four, _CMP_LT_OQ);
                if (_mm256_movemask_ps(mask) == 0) break;

                iterations = _mm256_add_epi32(iterations, _mm256_and_si256(_mm256_castps_si256(mask), _mm256_set1_epi32(1)));
                __m256 new_re = _mm256_add_ps(_mm256_sub_ps(re2, im2), c_real_v);
                z_im = _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(z_re, z_im)), c_imag_v);
                z_re = new_re;
            }

            int iters[8];
            _mm256_storeu_si256((__m256i*)iters, iterations);
            for (int k = 0; k < 8; k++) get_color(iters[k], &s->data[y][(x + k) * 3]);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // デフォルト値
    char *filename = "out.png";
    double cx = -0.745, cy = 0.1; // 人気のズームスポット
    double range = 2.5;

    if (argc >= 5) {
        filename = argv[1];
        cx = atof(argv[2]);
        cy = atof(argv[3]);
        range = atof(argv[4]);
    } else {
        printf("Usage: %s [filename] [center_x] [center_y] [range]\n", argv[0]);
        printf("Using default values...\n");
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

    struct IHDR ihdr = { .width = WIDTH, .height = HEIGHT, .bit_depth = 8, .color_type = PNG_COLOR_TYPE_RGB };
    write_png(filename, &ihdr, NULL, NULL, 0, data);

    for (size_t y = 0; y < HEIGHT; y++) free(data[y]);
    free(data);
    printf("Saved to %s (Center: %f, %f, Range: %f)\n", filename, cx, cy, range);
    return 0;
}
