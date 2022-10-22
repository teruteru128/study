
#include "pngheaders.h"
#include <stdio.h>

int gengradation(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "出力ファイル\n");
        return 1;
    }
    png_color c1 = { 0 };
    c1.red = 249;
    c1.green = 0;
    c1.blue = 149;
    png_color c2 = { 0 };
    c2.red = 255;
    c2.green = 147;
    c2.blue = 236;
    struct IHDR ihdr = { 0 };
    ihdr.width = 1920 * 4;
    ihdr.height = 1080 * 4;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte **data = malloc(sizeof(png_byte *) * ihdr.height);
    ssize_t len2 = 0;
    unsigned char colors[6];
    ssize_t a = getrandom(colors, 6, 0);
    if (a < 6)
    {
        return 1;
    }
    size_t rawwidthsize = sizeof(png_byte) * ihdr.width * 3;
    for (size_t y = 0; y < ihdr.height; y++)
    {
        data[y] = malloc(rawwidthsize);
        for (size_t x = 0; x < rawwidthsize; x += 3)
        {
            data[y][x + 0] = (png_byte)(((double)colors[0] * (ihdr.height - y)
                                         + colors[3] * y)
                                        / ihdr.height);
            data[y][x + 1] = (png_byte)(((double)colors[1] * (ihdr.height - y)
                                         + colors[4] * y)
                                        / ihdr.height);
            data[y][x + 2] = (png_byte)(((double)colors[2] * (ihdr.height - y)
                                         + colors[5] * y)
                                        / ihdr.height);
            // data[y][x + 3] = (png_byte)(255. * (ihdr.width - (x / 4)) /
            // ihdr.width);
        }
    }
    write_png(argv[1], &ihdr, NULL, NULL, 0, data);
error:
    for (size_t y = 0; y < ihdr.height; y++)
    {
        free(data[y]);
    }
    free(data);

    return 0;
}
