
#include "pngheaders.h"
#include <png.h>
#include <stdlib.h>
#include <sys/random.h>

#define BUFFER_SIZE 480
#define WIDTH 1920
#define HEIGHT 1080

/**
 * @brief カラーパレットを2個にして見るテスト
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char const *argv[])
{
    struct IHDR ihdr = { 0 };
    ihdr.width = WIDTH;
    ihdr.height = HEIGHT;
    ihdr.bit_depth = 1;
    ihdr.color_type = PNG_COLOR_TYPE_GRAY;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_color pallets[2] = { 0 };
    // #ffffff
    pallets[0].red = pallets[0].green = pallets[0].blue = 255;
    // #e5d75e
    pallets[1].red = 229;
    pallets[1].green = 215;
    pallets[1].blue = 94;
    png_byte **data = malloc(sizeof(png_byte *) * HEIGHT);
    for (size_t y = 0; y < HEIGHT; y++)
    {
        data[y] = malloc(sizeof(png_byte) * WIDTH/8);
    }
    unsigned char buf[BUFFER_SIZE];
    ssize_t successsize = 0;
    for (size_t y = 0; y < HEIGHT; y++)
    {
        successsize = getrandom(buf, BUFFER_SIZE, 0);
        if (successsize < BUFFER_SIZE)
        {
            fprintf(stderr, "fail!");
            break;
        }
        for (size_t x = 0; x < 240; x++)
        {
            // BUFFER_SIZE が 240 のとき
            // data[y][x] = (buf[x >> 3] >> (x & 7)) & 1;
            // BUFFER_SIZE が 480 のとき
            // data[y][x] = ((buf[x >> 3] >> (x & 6)) & 3) < 3;
            data[y][x] = 0xff;
        }
    }

    // write_png("map5.png", &ihdr, NULL, pallets, 2, data);
    write_png("png_pallets_sample7.png", &ihdr, NULL, NULL, 0, data);
    for (size_t y = 0; y < HEIGHT; y++)
    {
        free(data[y]);
        data[y] = NULL;
    }
    free(data);
    return 0;
}
