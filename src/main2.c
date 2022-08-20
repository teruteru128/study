
#define _GNU_SOURCE
#include "config.h"

#include <inttypes.h>
#include <math.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void e(png_struct *a, png_const_charp b)
{
    png_get_error_ptr(a);
    printf("%s\n", b);
}

// 画像ヘッダ
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
// 物理ピクセル解像度
struct pHYs
{
    int32_t res_x;
    int32_t res_y;
    int type;
};
static png_byte **row_pointers = NULL;

static int read_png(const char *inpath, struct IHDR *ihdr, struct pHYs *phys)
{
    FILE *infp = fopen(inpath, "rb");
    if (infp == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    png_error_ptr p;
    png_struct *png_ptr
        = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, e, e);
    if (!png_ptr)
        return (EXIT_FAILURE);

    png_info *info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return (EXIT_FAILURE);
    }

    png_info *end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return (EXIT_FAILURE);
    }
    /* Set error handling if you are using the setjmp/longjmp method (this
     * is the normal method of doing things with libpng).  REQUIRED unless
     * you set up your own error handlers in the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr.
         */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(infp);
        /* If we get here, we had a problem reading the file. */
        return (EXIT_FAILURE);
    }
    png_init_io(png_ptr, infp);
    png_set_sig_bytes(png_ptr, 0);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    if (png_get_IHDR(png_ptr, info_ptr, &ihdr->width, &ihdr->height,
                     &ihdr->bit_depth, &ihdr->color_type,
                     &ihdr->interlace_method, &ihdr->compression_method,
                     &ihdr->filter_method))
    {
        printf("image header: %" PRId32 " %" PRId32 "\n", ihdr->width,
               ihdr->height);
        printf("%d %d %d %d %d\n", ihdr->bit_depth, ihdr->color_type,
               ihdr->interlace_method, ihdr->compression_method,
               ihdr->filter_method);
    }
    else
    {
        printf("png_get_IHDR failed\n");
    }
    if (png_get_pHYs(png_ptr, info_ptr, &phys->res_x, &phys->res_y,
                     &phys->type))
    {
        printf("physical pixel resolution: %" PRId32 " %" PRId32 " %d\n",
               phys->res_x, phys->res_y, phys->type);
    }
    else
    {
        printf("png_get_pHYs failed\n");
    }

    // dupliacte rows
    png_byte **original_row_pointers = png_get_rows(png_ptr, info_ptr);
    row_pointers = malloc(sizeof(png_byte *) * ihdr->height);
    size_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
    for (size_t y = 0; y < ihdr->height; y++)
    {
        row_pointers[y] = malloc(rowsize);
        memcpy(row_pointers[y], original_row_pointers[y], rowsize);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(infp);
    return 0;
}

static int write_png(const char *outpath, struct IHDR *ihdr, struct pHYs *phys)
{
    FILE *outfp = fopen(outpath, "wb");
    png_struct *outpng_ptr
        = png_create_write_struct(PNG_LIBPNG_VER_STRING, e, e, e);
    if (outpng_ptr == NULL)
    {
        printf("outpng_ptr is null\n");
        fclose(outfp);
        return EXIT_FAILURE;
    }
    png_info *outinfo_ptr = png_create_info_struct(outpng_ptr);
    if (outinfo_ptr == NULL)
    {
        printf("outinfo_ptr is null\n");
        png_destroy_write_struct(&outpng_ptr, NULL);
        fclose(outfp);
        return EXIT_FAILURE;
    }
    if (setjmp(png_jmpbuf(outpng_ptr)))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr.
         */
        png_destroy_read_struct(&outpng_ptr, &outinfo_ptr, NULL);
        fclose(outfp);
        /* If we get here, we had a problem reading the file. */
        return (EXIT_FAILURE);
    }
    png_init_io(outpng_ptr, outfp);
    /*
     * PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
     * PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
     */
    png_set_IHDR(outpng_ptr, outinfo_ptr, ihdr->width, ihdr->height,
                 ihdr->bit_depth, ihdr->color_type, ihdr->interlace_method,
                 ihdr->compression_method, ihdr->filter_method);
    png_set_pHYs(outpng_ptr, outinfo_ptr, phys->res_x, phys->res_y,
                 phys->type);
    png_set_rows(outpng_ptr, outinfo_ptr, row_pointers);
    png_write_png(outpng_ptr, outinfo_ptr, 0, NULL);
    png_destroy_write_struct(&outpng_ptr, &outinfo_ptr);
    fclose(outfp);
    return 0;
}

/**
 * @brief pngファイルを読み込んで解像度を書き換えてファイルに書き出す
 *
 */
int hiho(int argc, char **argv, const char **envp)
{
    struct IHDR ihdr = { 0 };
    struct pHYs phys = { 0 };
    const char inpath[]
        = "/mnt/g/iandm/image/waifu2x/pixiv.net/"
          "32668232_p0(UpRGB)(noise_scale)(Level0)(x8.000000).png";
    const char outpath[] = "32668232_p0_350dpi.png";
    read_png(inpath, &ihdr, &phys);
    printf("read ok\n");
    // 350 dpi to dots per meter
    phys.res_x = phys.res_y = floor((350 * 10000) / 254.);
    write_png(outpath, &ihdr, &phys);
    for (size_t y = 0; y < ihdr.height; y++)
    {
        free(row_pointers[y]);
        row_pointers[y] = NULL;
    }
    free(row_pointers);

    return 0;
}
