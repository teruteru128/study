
#include "pngheaders.h"
#include <complex.h>
#include <immintrin.h>
#include <png.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 2560
#define HEIGHT 1920
#define MAX_ITER 1000
#define THREAD_COUNT 16  // 使用するスレッド数

// 全スレッド共通の設定
typedef struct {
    png_byte **data;
    float complex left_top;
    float unit;
} SharedData;

// 各スレッド固有の作業範囲
typedef struct {
    int thread_id;
    int start_y;
    int end_y;
    SharedData *shared;
} ThreadArg;

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

// 反復回数からRGBへの変換（簡易HSV）
void get_color(int iter, unsigned char *rgb) {
    if (iter == MAX_ITER) {
        rgb[0] = rgb[1] = rgb[2] = 0; // 集合内部は黒
        return;
    }
    // 反復回数に基づいた青〜白のグラデーション例
    rgb[0] = (iter * 7) % 255;
    rgb[1] = (iter * 3) % 255;
    rgb[2] = (iter * 11) % 255;
}

void *thread_func(void *arg) {
    ThreadArg *t_arg = (ThreadArg *)arg;
    SharedData *s = t_arg->shared;

    for (int y = t_arg->start_y; y < t_arg->end_y; y++) {
        float c_imag = cimagf(s->left_top) - (float)y * s->unit;
        __m256 c_imag_v = _mm256_set1_ps(c_imag);

        for (int x = 0; x < WIDTH; x += 8) { // 8ピクセルずつ処理
            // 各ピクセルの c_real をセット
            __m256 c_real_v = _mm256_set_ps(
                crealf(s->left_top) + (x+7) * s->unit, crealf(s->left_top) + (x+6) * s->unit,
                crealf(s->left_top) + (x+5) * s->unit, crealf(s->left_top) + (x+4) * s->unit,
                crealf(s->left_top) + (x+3) * s->unit, crealf(s->left_top) + (x+2) * s->unit,
                crealf(s->left_top) + (x+1) * s->unit, crealf(s->left_top) + (x+0) * s->unit
            );

            __m256 z_re = _mm256_setzero_ps();
            __m256 z_im = _mm256_setzero_ps();
            __m256i iterations = _mm256_setzero_si256();
            __m256 four = _mm256_set1_ps(4.0f);

            for (int i = 0; i < MAX_ITER; i++) {
                __m256 re2 = _mm256_mul_ps(z_re, z_re);
                __m256 im2 = _mm256_mul_ps(z_im, z_im);
                __m256 dist2 = _mm256_add_ps(re2, im2);

                // |z|^2 < 4.0 の要素だけ 1 になるマスク
                __m256 mask = _mm256_cmp_ps(dist2, four, _CMP_LT_OQ);
                int mask_int = _mm256_movemask_ps(mask);
                if (mask_int == 0) break; // 全員発散したら終了

                // 反復回数のインクリメント
                iterations = _mm256_add_epi32(iterations, _mm256_and_si256(_mm256_castps_si256(mask), _mm256_set1_epi32(1)));

                // z = z^2 + c => re = re^2 - im^2 + c_re, im = 2*re*im + c_im
                __m256 new_re = _mm256_add_ps(_mm256_sub_ps(re2, im2), c_real_v);
                z_im = _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(2.0f), _mm256_mul_ps(z_re, z_im)), c_imag_v);
                z_re = new_re;
            }

            // 計算結果の書き込み
            int iters[8];
            _mm256_storeu_si256((__m256i*)iters, iterations);
            for (int k = 0; k < 8; k++) {
                get_color(iters[k], &s->data[y][(x + k) * 3]);
            }
        }
    }
    return NULL;
}

int main() {
    // 1. 画像メモリの確保
    png_byte **data = malloc(HEIGHT * sizeof(png_byte *));
    for (size_t y = 0; y < HEIGHT; y++) data[y] = malloc(3 * WIDTH);

    // 2. 共有データの設定
    SharedData shared = {
        .data = data,
        .left_top = -2.0f + 1.0f * I,
        .unit = 2.0f / HEIGHT
    };

    // 3. スレッドの生成と実行
    pthread_t threads[THREAD_COUNT];
    ThreadArg args[THREAD_COUNT];
    int rows_per_thread = HEIGHT / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
        args[i].thread_id = i;
        args[i].shared = &shared;
        args[i].start_y = i * rows_per_thread;
        // 最後のスレッドが余り（端数）をカバーするように設定
        args[i].end_y = (i == THREAD_COUNT - 1) ? HEIGHT : (i + 1) * rows_per_thread;

        if (pthread_create(&threads[i], NULL, thread_func, &args[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // 4. すべてのスレッドが終了するのを待つ
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    // 5. PNG保存
    struct IHDR ihdr = { .width = WIDTH, .height = HEIGHT, .bit_depth = 8, 
                         .color_type = PNG_COLOR_TYPE_RGB, .interlace_method = PNG_INTERLACE_NONE,
                         .compression_method = PNG_COMPRESSION_TYPE_DEFAULT, .filter_method = PNG_NO_FILTERS };
    write_png("out.png", &ihdr, NULL, NULL, 0, data);

    // 6. 後片付け
    for (size_t y = 0; y < HEIGHT; y++) free(data[y]);
    free(data);

    printf("Parallel calculation complete.\n");
    return 0;
}
