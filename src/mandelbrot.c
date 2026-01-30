
#include "pngheaders.h"
#include <complex.h>
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

// スレッドが実行するメイン関数
void *thread_func(void *arg) {
    ThreadArg *t_arg = (ThreadArg *)arg;
    SharedData *s = t_arg->shared;

    for (int y = t_arg->start_y; y < t_arg->end_y; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float complex c = s->left_top + (float)x * s->unit - (float)y * s->unit * I;
            int is_inside = mandelbrot(c);
            unsigned char color = is_inside ? 0x00 : 0xFF;

            s->data[y][x*3 + 0] = color;
            s->data[y][x*3 + 1] = color;
            s->data[y][x*3 + 2] = color;
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
