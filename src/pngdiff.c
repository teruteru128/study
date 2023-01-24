
#include "pngheaders.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    if (argc < 4)
    {
        return 1;
    }
    char before_path[PATH_MAX];
    char after_path[PATH_MAX];
    char output_path[PATH_MAX];
    strncpy(before_path, argv[1], PATH_MAX);
    strncpy(after_path, argv[2], PATH_MAX);
    strncpy(output_path, argv[3], PATH_MAX);
    struct IHDR before_ihdr;
    struct IHDR after_ihdr;
    png_byte **before_data = NULL;
    png_byte **after_data = NULL;
    if (read_png(argv[1], &before_ihdr, NULL, NULL, NULL, &before_data)
            != EXIT_SUCCESS
        || read_png(argv[2], &after_ihdr, NULL, NULL, NULL, &after_data)
               != EXIT_SUCCESS)
    {
        return 1;
    }
    if (before_ihdr.height != after_ihdr.height
        || before_ihdr.width != after_ihdr.width)
    {
        for (size_t y = 0; y < before_ihdr.height; y++)
        {
            free(before_data[y]);
            before_data[y] = NULL;
        }
        before_data = NULL;
        for (size_t y = 0; y < after_ihdr.height; y++)
        {
            free(after_data[y]);
            after_data[y] = NULL;
        }
        after_data = NULL;
        return 2;
    }
    struct IHDR output_ihdr = before_ihdr;
    png_byte **output_data = malloc(sizeof(png_byte *) * output_ihdr.height);
    for (size_t y = 0; y < output_ihdr.height; y++)
    {
        output_data[y] = malloc(sizeof(png_byte) * 4 * output_ihdr.width);
        for (size_t x = 0; x < output_ihdr.width; x++)
        {
            if (memcmp(&before_data[y][x * 4], &after_data[y][x * 4], 4) != 0)
            {
                memcpy(&output_data[y][x * 4], &before_data[y][x * 4], 4);
            }
            else
            {
                output_data[y][x * 4 + 0] = 255;
                output_data[y][x * 4 + 1] = 255;
                output_data[y][x * 4 + 2] = 255;
                output_data[y][x * 4 + 3] = 0;
            }
        }
    }
    if (write_png(output_path, &output_ihdr, NULL, NULL, 0, output_data)
        != EXIT_SUCCESS)
    {
        return 1;
    }
    for (size_t y = 0; y < output_ihdr.height; y++)
    {
        free(output_data[y]);
        output_data[y] = NULL;
        free(before_data[y]);
        before_data[y] = NULL;
        free(after_data[y]);
        after_data[y] = NULL;
    }
    free(output_data);
    free(before_data);
    free(after_data);
    output_data = NULL;
    before_data = NULL;
    after_data = NULL;

    return 0;
}
