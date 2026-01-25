
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <png.h>

// 構造体で画像を管理
typedef struct {
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
} PNGImage;

// PNG読み込み
void read_png(const char *filename, PNGImage *img) {
    FILE *fp = fopen(filename, "rb");
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    setjmp(png_jmpbuf(png));
    png_init_io(png, fp);
    png_read_info(png, info);

    img->width = png_get_image_width(png, info);
    img->height = png_get_image_height(png, info);
    img->color_type = png_get_color_type(png, info);
    img->bit_depth = png_get_bit_depth(png, info);

    // RGBAに変換を強制
    if (img->color_type == PNG_COLOR_TYPE_RGB) png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);
    png_read_update_info(png, info);

    img->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img->height);
    for (int y = 0; y < img->height; y++) {
        img->row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
    }
    png_read_image(png, img->row_pointers);
    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
}

// PNG書き出し
void write_png(const char *filename, PNGImage *img) {
    FILE *fp = fopen(filename, "wb");
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    setjmp(png_jmpbuf(png));
    png_init_io(png, fp);

    png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    png_write_image(png, img->row_pointers);
    png_write_end(png, NULL);
    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

int main(int argc, char *argv[]) {
    if (argc < 8) {
        printf("使用法: %s img1.png img2.png out.png R G B A\n", argv[0]);
        printf("※RGBAは 0-255。A=0で透明。\n");
        return 1;
    }

    PNGImage img1, img2, out;
    read_png(argv[1], &img1);
    read_png(argv[2], &img2);

    if (img1.width != img2.width || img1.height != img2.height) {
        fprintf(stderr, "画像サイズが一致しません。\n");
        return 1;
    }

    // 出力用画像の設定
    out = img1;
    out.row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * out.height);
    int bg_r = atoi(argv[4]), bg_g = atoi(argv[5]), bg_b = atoi(argv[6]), bg_a = atoi(argv[7]);

    for (int y = 0; y < out.height; y++) {
        out.row_pointers[y] = (png_byte*)malloc(out.width * 4);
        for (int x = 0; x < out.width; x++) {
            png_bytep px1 = &(img1.row_pointers[y][x * 4]);
            png_bytep px2 = &(img2.row_pointers[y][x * 4]);
            png_bytep pxO = &(out.row_pointers[y][x * 4]);

            // 画素の比較 (RGBのみ)
            if (px1[0] == px2[0] && px1[1] == px2[1] && px1[2] == px2[2]) {
                pxO[0] = bg_r; pxO[1] = bg_g; pxO[2] = bg_b; pxO[3] = bg_a;
            } else {
                // 差がある場合は img2 の色を表示
                pxO[0] = px2[0]; pxO[1] = px2[1]; pxO[2] = px2[2]; pxO[3] = 255;
            }
        }
    }

    write_png(argv[3], &out);
    printf("差分画像を保存しました: %s\n", argv[3]);
    return 0;
}
