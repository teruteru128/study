
#pragma once
#ifndef PNG_HEADERS_H
#define PNG_HEADERS_H 1

#include <png.h>
#include <stdint.h>

// 画像ヘッダチャンク
struct IHDR
{
    int32_t width;
    int32_t height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
};
// 物理ピクセル解像度チャンク
struct pHYs
{
    int32_t res_x;
    int32_t res_y;
    int type;
};

__BEGIN_DECLS

int read_png(const char *inpath, struct IHDR *ihdr, struct pHYs *phys,
             png_colorp *palettes, int *num_palette, png_byte ***row_pointers);

int write_png(const char *outpath, struct IHDR *ihdr, struct pHYs *phys,
              png_colorp palettes, int num_palette, png_byte **row_pointers);
__END_DECLS

#endif
