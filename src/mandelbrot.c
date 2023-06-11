
#define _GNU_SOURCE 1

#include "pngheaders.h"
#include <complex.h>
#include <math.h>
#include <png.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct arg
{
    size_t x;
    size_t y;
    float complex c;
    int result;
};

void *func(void *a)
{
    struct arg *args = (struct arg *)a;
    float complex z = 0;
    const float complex c = args->c;
    for (size_t i = 0; i < 1000; i++)
    {
        z = z * z + c;
        if (cabsf(z) >= 3)
        {
            break;
        }
    }
    if (cimag(z) <= 1)
    {
        args->result = 1;
    }
    else
    {
        args->result = 0;
    }
    return a;
}

#define WIDTH 2560
#define HEIGHT 1920

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char const *argv[])
{
    size_t width = WIDTH;
    size_t height = HEIGHT;
    //
    float complex hidariue = -2. + 1.i;
    const float unit = 2.1 / HEIGHT;
    float complex z = 0;
    float complex c = 0;
    size_t y = 0;
    size_t i = 0;
    struct IHDR ihdr = { 0 };
    ihdr.width = WIDTH;
    ihdr.height = HEIGHT;
    ihdr.bit_depth = 8;
    ihdr.color_type = PNG_COLOR_TYPE_RGB;
    ihdr.interlace_method = PNG_INTERLACE_NONE;
    ihdr.compression_method = PNG_COMPRESSION_TYPE_DEFAULT;
    ihdr.filter_method = PNG_NO_FILTERS;
    png_byte **data = malloc(HEIGHT * sizeof(png_byte *));
    for (size_t x = 0; x < HEIGHT; x++)
    {
        data[x] = malloc(sizeof(png_byte) * 3 * WIDTH);
    }
    int result = 0;
    for (size_t x = 0; x < WIDTH; x++)
    {
        for (y = 0; y < HEIGHT; y++)
        {
            z = 0;
            c = hidariue + unit * x - unit * y * I;
            for (i = 0; i < 1000; i++)
            {
                z = z * z + c;
                if (cimagf(z) >= 3)
                {
                    break;
                }
            }
            result = cimagf(z) >= 1;
            data[y][x*3 + 0] = 0xff * result;
            data[y][x*3 + 1] = 0xff * result;
            data[y][x*3 + 2] = 0xff * result;
        }
    }
    int ret = 0;

    ret = write_png("out.png", &ihdr, NULL, NULL, 0, data);
    if (ret == 0)
    {
        printf("OK\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return 0;
}
