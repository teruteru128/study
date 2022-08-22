
#include <inttypes.h>
#include <math.h>
#include <png.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

static int read_png(const char *inpath, struct IHDR *ihdr, struct pHYs *phys,
                    png_byte ***row_pointers)
{
    if (inpath == NULL || row_pointers == NULL)
    {
        return 1;
    }
    FILE *fp = fopen(inpath, "rb");
    if (fp == NULL)
    {
        perror("fopen");
        return EXIT_FAILURE;
    }
    png_error_ptr p;
    png_struct *png_ptr
        = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(fp);
        return (EXIT_FAILURE);
    }

    png_info *info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
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
        fclose(fp);
        /* If we get here, we had a problem reading the file. */
        return (EXIT_FAILURE);
    }
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 0);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    if (ihdr)
    {
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
    }
    if (phys)
    {
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
    }

    // dupliacte rows
    png_byte **original_row_pointers = png_get_rows(png_ptr, info_ptr);
    *row_pointers = malloc(sizeof(png_byte *) * ihdr->height);
    size_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
    for (size_t y = 0; y < ihdr->height; y++)
    {
        (*row_pointers)[y] = malloc(rowsize);
        memcpy((*row_pointers)[y], original_row_pointers[y], rowsize);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return 0;
}

static int write_png(const char *outpath, struct IHDR *ihdr, struct pHYs *phys,
                     png_byte **row_pointers)
{
    FILE *fp = fopen(outpath, "wb");
    png_struct *png_ptr
        = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        printf("png_ptr is null\n");
        fclose(fp);
        return EXIT_FAILURE;
    }
    png_info *info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        printf("info_ptr is null\n");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return EXIT_FAILURE;
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr.
         */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        /* If we get here, we had a problem reading the file. */
        return (EXIT_FAILURE);
    }
    png_init_io(png_ptr, fp);
    /*
     * PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
     * PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
     */
    png_set_IHDR(png_ptr, info_ptr, ihdr->width, ihdr->height, ihdr->bit_depth,
                 ihdr->color_type, ihdr->interlace_method,
                 ihdr->compression_method, ihdr->filter_method);
    png_set_pHYs(png_ptr, info_ptr, phys->res_x, phys->res_y, phys->type);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return 0;
}

/**
 * @brief pngファイルを読み込んで解像度を書き換えてファイルに書き出す
 *
 */
int main(int argc, char const *argv[], const char **envp)
{
    struct IHDR ihdr = { 0 };
    struct pHYs phys = { 0 };
    const char inpath[] = "/mnt/g/iandm/image/waifu2x/pixiv.net/"
                          "52629088_p0(RGB)(scale)(x2.000000)(16bit).png";
    const char outpath[]
        = "/mnt/g/iandm/image/waifu2x/pixiv.net/52629088_p0_350dpi.png";
    png_byte **row_pointers = NULL;
    read_png(inpath, &ihdr, &phys, &row_pointers);
    printf("read ok\n");
    // 350 dpi to dots per meter
    phys.res_x = phys.res_y = floor((350 * 10000) / 254.);
    write_png(outpath, &ihdr, &phys, row_pointers);
    for (size_t y = 0; y < ihdr.height; y++)
    {
        free(row_pointers[y]);
        row_pointers[y] = NULL;
    }
    free(row_pointers);
    return 0;
}
