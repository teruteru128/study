
#include "pngheaders.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pathcon(char *out, size_t cap, const char *in1, const char *in2)
{
    out[0] = '\0';
    size_t len1 = strlen(in1);
    size_t len2 = strlen(in2);
    size_t cap_nokori = cap;
    strncat(out, in1, cap_nokori);
    out += len1;
    cap_nokori -= len1;
    strncat(out, in2, cap_nokori);
}

void con(png_byte **output_data, png_byte ***input_data, size_t width)
{
    for (size_t y = 0; y < 1200; y++)
    {
        memcpy(output_data[y] + sizeof(png_byte) * 4 * 822, input_data[0][y],
               sizeof(png_byte) * 4 * 822);
    }
    for (size_t y = 0; y < 1200; y++)
    {
        memcpy(output_data[y] + 0, input_data[1][y],
               sizeof(png_byte) * 4 * 822);
    }
}

int main(int argc, char const *argv[])
{
    const size_t out_width = 822 * 2;
    const size_t out_height = 1200;
    const char inputfiledir[]
        = "/mnt/g/iandm/image/shonenjumpplus.com/13933686331749163174/";
    const char inputfilenamearray[61][10] = { "" };
    char outfilename[PATH_MAX] = "";
    png_color *palettes[4] = { NULL };
    int num_palette[4] = { 0 };

    struct IHDR in_ihdr[4] = { 0 };
    struct IHDR out_ihdr = { 0 };
    png_byte **input_data[4] = { NULL };
    png_byte **input_rgb[4] = { NULL };
    char inputpath[PATH_MAX];
    // 書き出しキャンバス
    png_byte **output_data = malloc(sizeof(png_byte *) * out_height);
    for (size_t y = 0; y < out_height; y++)
    {
        output_data[y] = malloc(sizeof(png_byte) * 4 * out_width);
    }
    for (size_t i = 1; i < 60; i += 2)
    {
        snprintf(inputpath, PATH_MAX, "%spage%02zu.png", inputfiledir, i);
        read_png(inputpath, &in_ihdr[0], NULL, NULL, NULL, &input_data[0]);
        snprintf(inputpath, PATH_MAX, "%spage%02zu.png", inputfiledir, i + 1);
        read_png(inputpath, &in_ihdr[1], NULL, NULL, NULL, &input_data[1]);
        // 画像ヘッダデータ作成
        out_ihdr.width = out_width;
        out_ihdr.height = out_height;
        out_ihdr.bit_depth = in_ihdr[0].bit_depth;
        out_ihdr.color_type = PNG_COLOR_TYPE_RGBA;
        out_ihdr.interlace_method = in_ihdr[0].interlace_method;
        out_ihdr.compression_method = in_ihdr[0].compression_method;
        out_ihdr.filter_method = in_ihdr[0].filter_method;
        // 結合
        con(output_data, input_data, out_width);
        snprintf(outfilename, PATH_MAX, "%swork/page%02zu-%02zu.png",
                 inputfiledir, i, i + 1);
        // 片付け
        for (size_t y = 0; y < in_ihdr[0].height; y++)
        {
            free(input_data[0][y]);
            input_data[0][y] = NULL;
        }
        input_data[0] = NULL;
        for (size_t y = 0; y < in_ihdr[1].height; y++)
        {
            free(input_data[1][y]);
            input_data[1][y] = NULL;
        }
        input_data[1] = NULL;

        // 書き出し
        write_png(outfilename, &out_ihdr, NULL, NULL, 0, output_data);
    }

    // 後片付け
    for (size_t y = 0; y < out_height; y++)
    {
        free(output_data[y]);
        output_data[y] = NULL;
    }
    free(output_data);
    output_data = NULL;

    return 0;
}
