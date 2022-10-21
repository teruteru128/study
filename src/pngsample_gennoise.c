
#include "pngheaders.h"
#include <stdlib.h>
#include <sys/random.h>

int gennoise()
{
    struct IHDR ihdr = { 0 };
    ihdr.width = 1920;
    ihdr.height = 1080;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte **data = malloc(sizeof(png_byte *) * 1080);
    ssize_t size = 0;
    size_t x = 0;
    const size_t byteswidth = sizeof(png_byte) * 3 * 1920;
    for (size_t y = 0; y < 1080; y++)
    {
        data[y] = malloc(byteswidth);
        size = getrandom(data[y], byteswidth, 0);
        for (x = 0; x < byteswidth; x++)
        {
            data[y][x] = 255 - (png_byte)(data[y][x] * 0.1875);
        }
    }

    write_png("noise1.png", &ihdr, NULL, NULL, 0, data);

    for (size_t y = 0; y < 1080; y++)
    {
        free(data[y]);
    }
    free(data);
    return 0;
}