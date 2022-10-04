
#include "pngheaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    size_t offset = 0;
    for (size_t y = 0; y < 1385; y++)
    {
        memcpy(output_data[y + offset], input_data[0][y],
               sizeof(png_byte) * 3 * width);
    }
    offset += 1385;
    for (size_t y = 0; y < 1335; y++)
    {
        memcpy(output_data[y + offset], input_data[1][y],
               sizeof(png_byte) * 3 * width);
    }
    offset += 1335;
    for (size_t y = 0; y < 1240; y++)
    {
        memcpy(output_data[y + offset], input_data[2][y],
               sizeof(png_byte) * 3 * width);
    }
    offset += 1240;
    for (size_t y = 0; y < 1390; y++)
    {
        memcpy(output_data[y + offset], input_data[3][y],
               sizeof(png_byte) * 3 * width);
    }
    offset += 1390;
}

int main(int argc, char const *argv[])
{
    const size_t width = 2168;
    const size_t height = 1385 + 1335 + 1240 + 1390;
    const char inputfiledir[] = "/mnt/g/iandm/image/twitter.com/F/A/";
    const char *inputfilenamearray[]
        = { "Fay9lEfaAAAJXrP.png", "Fay99f8agAAFsYS.png",
            "Fay9_iTagAAC6bn.png", "Fay-B87aUAAN-5i.png" };
    const char outfilename[] = "chisataki1200dpi.png";
    png_color *palettes[4] = { NULL };
    int num_palette[4] = { 0 };

    struct IHDR in_ihdr[4] = { 0 };
    struct IHDR out_ihdr = { 0 };
    png_byte **input_data[4] = { NULL };
    png_byte **input_rgb[4] = { NULL };
    char inputpath[PATH_MAX];
    // 画像4枚読み込み
    for (size_t i = 0; i < 4; i++)
    {
        pathcon(inputpath, PATH_MAX, inputfiledir, inputfilenamearray[i]);
        if (read_png(inputpath, &in_ihdr[i], NULL, &palettes[i],
                     num_palette + i, &input_data[i]))
        {
            return EXIT_FAILURE;
        }
        input_rgb[i] = malloc(sizeof(png_byte *) * in_ihdr[i].height);
        for (size_t y = 0; y < in_ihdr[i].height; y++)
        {
            input_rgb[i][y] = malloc(sizeof(png_byte) * 3 * in_ihdr[i].width);
            for (size_t x = 0; x < in_ihdr[i].width; x++)
            {
                input_rgb[i][y][x * 3 + 0]
                    = palettes[i][input_data[i][y][x]].red;
                input_rgb[i][y][x * 3 + 1]
                    = palettes[i][input_data[i][y][x]].green;
                input_rgb[i][y][x * 3 + 2]
                    = palettes[i][input_data[i][y][x]].blue;
            }
        }

    }

    // 書き出しキャンバス
    png_byte **output_data = malloc(sizeof(png_byte *) * height);
    for (size_t y = 0; y < height; y++)
    {
        output_data[y] = malloc(sizeof(png_byte) * 3 * width);
    }
    // 結合
    con(output_data, input_rgb, width);

    // 画像データ作成
    out_ihdr.width = width;
    out_ihdr.height = height;
    out_ihdr.bit_depth = in_ihdr[0].bit_depth;
    out_ihdr.color_type = PNG_COLOR_TYPE_RGB;
    out_ihdr.interlace_method = in_ihdr[0].interlace_method;
    out_ihdr.compression_method = in_ihdr[0].compression_method;
    out_ihdr.filter_method = in_ihdr[0].filter_method;
    struct pHYs phys = { 0 };
    phys.res_x = phys.res_y = floor((1200 * 10000) / 254.);
    phys.type = PNG_RESOLUTION_METER;

    // 書き出し
    write_png(outfilename, &out_ihdr, &phys, NULL, 0, output_data);

    // 後片付け
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t y = 0; y < in_ihdr[i].height; y++)
        {
            free(input_data[i][y]);
            input_data[i][y] = NULL;
            free(input_rgb[i][y]);
            input_rgb[i][y] = NULL;
        }
        free(palettes[i]);
    }

    return 0;
}
