
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h> // getrandom(2) のために必要
#include <png.h>

// getrandom(2) を使って指定バイト分の乱数をバッファに安全に満たす関数
void fill_random_bytes(void *buf, size_t buflen) {
    size_t bytes_filled = 0;
    while (bytes_filled < buflen) {
        // 残りの未充填バッファに乱数を書き込む
        ssize_t res = getrandom((char *)buf + bytes_filled, buflen - bytes_filled, GRND_NONBLOCK);
        
        if (res > 0) {
            bytes_filled += res;
        } else {
            // エラーまたはエントロピー一時不足の場合は少し待つか、最悪フォールバック
            // (通常、非暗号用の十分なプールがあればGRND_NONBLOCKでも即座に取得できます)
            perror("getrandom failed or non-blocking pool empty. Retrying...");
        }
    }
}

// PNG書き出し関数
int save_png(const char *filename, int width, int height, png_bytep *row_pointers) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return -1; }

    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_write_struct(&png, NULL); fclose(fp); return -1; }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return -1;
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <width> <height> <tile_size> <num_tiles> <output.png>\n", argv[0]);
        fprintf(stderr, "  num_tiles: 1, 2, 4, 8 (0 for complete random)\n");
        return 1;
    }

    int width     = atoi(argv[1]);
    int height    = atoi(argv[2]);
    int tile_size = atoi(argv[3]);
    int num_tiles = atoi(argv[4]);
    const char *filename = argv[5];

    // メイン画像のメモリ確保 (RGB: 1ピクセル3バイト)
    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte *)malloc(3 * width);
    }

    // 0が指定されたら完全ランダムノイズ
    if (num_tiles == 0) {
        for (int y = 0; y < height; y++) {
            // 各行のバッファ全体（width * 3バイト）を一括して getrandom で埋める
            fill_random_bytes(row_pointers[y], 3 * width);
        }
    } else {
        // 1. 指定された個数のノイズタイル（テンプレート）を作成
        png_byte **tiles = (png_byte **)malloc(sizeof(png_byte *) * num_tiles);
        size_t tile_bytes = 3 * tile_size * tile_size;
        for (int t = 0; t < num_tiles; t++) {
            tiles[t] = (png_byte *)malloc(tile_bytes);
            // タイルのバッファを getrandom で丸ごと埋める
            fill_random_bytes(tiles[t], tile_bytes);
        }

        // 2. メイン画像にタイルを敷き詰める
        for (int ty = 0; ty < height; ty += tile_size) {
            for (int tx = 0; tx < width; tx += tile_size) {
                
                // タイル選択用のインデックスだけは通常のインライン処理（1バイトだけ取得）
                unsigned char rand_val;
                fill_random_bytes(&rand_val, 1);
                int tile_idx = rand_val % num_tiles;

                // タイルの中身をメイン画像にコピー（画像端のクリッピング処理含む）
                for (int y = 0; y < tile_size && (ty + y) < height; y++) {
                    for (int x = 0; x < tile_size && (tx + x) < width; x++) {
                        int src_idx = (y * tile_size + x) * 3;
                        int dest_x  = tx + x;
                        
                        row_pointers[ty + y][dest_x * 3 + 0] = tiles[tile_idx][src_idx + 0];
                        row_pointers[ty + y][dest_x * 3 + 1] = tiles[tile_idx][src_idx + 1];
                        row_pointers[ty + y][dest_x * 3 + 2] = tiles[tile_idx][src_idx + 2];
                    }
                }
            }
        }

        // タイルメモリの解放
        for (int t = 0; t < num_tiles; t++) free(tiles[t]);
        free(tiles);
    }

    // PNG保存
    if (save_png(filename, width, height, row_pointers) == 0) {
        printf("Success: %s generated using getrandom(2).\n", filename);
    } else {
        fprintf(stderr, "Error: Failed to save PNG.\n");
    }

    // メイン画像メモリの解放
    for (int y = 0; y < height; y++) free(row_pointers[y]);
    free(row_pointers);

    return 0;
}

