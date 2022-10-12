
#include "pngheaders.h"
#include <png.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 0x6000
#define HEIGHT WIDTH

int main(int argc, char const *argv[])
{
    struct IHDR ihdr = { 0 };
    ihdr.width = WIDTH;
    ihdr.height = WIDTH;
    ihdr.bit_depth = 1;
    ihdr.color_type = PNG_COLOR_TYPE_GRAY;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte *row = malloc(WIDTH / 8);
    memset(row, 255, WIDTH / 8);
    png_byte **data = malloc(sizeof(png_byte *) * HEIGHT);
    for (size_t i = 0; i < HEIGHT; i++)
    {
        data[i] = row;
    }
    write_png("unchi14.png", &ihdr, NULL, NULL, 0, data);
    free(data);
    free(row);

    return 0;
}
