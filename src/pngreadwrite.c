
#include "pngheaders.h"
#include <inttypes.h>
#include <png.h>
#include <stdlib.h>
#include <string.h>

int read_png(const char *inpath, struct IHDR *ihdr, struct pHYs *phys,
             png_colorp *palettes, int *num_palette, png_byte ***row_pointers)
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
            printf("image header: %" PRId32 " x %" PRId32 " ", ihdr->width,
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
    if (palettes && num_palette)
    {
        png_color *tmp = NULL;
        int num_work = 0;
        if (png_get_PLTE(png_ptr, info_ptr, &tmp, &num_work))
        {
            *palettes = malloc(sizeof(png_color) * num_work);
            memcpy(*palettes, tmp, sizeof(png_color) * num_work);
            *num_palette = num_work;
            printf("get PLTE %d\n", *num_palette);
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
    printf("rowsize : %zu\n", rowsize);
    for (size_t y = 0; y < ihdr->height; y++)
    {
        (*row_pointers)[y] = malloc(rowsize);
        memcpy((*row_pointers)[y], original_row_pointers[y], rowsize);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return 0;
}

int write_png(const char *outpath, struct IHDR *ihdr, struct pHYs *phys,
              png_colorp palettes, int num_palette, png_byte **row_pointers)
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
    printf("set IHDR %"PRId32" x %"PRId32"\n", ihdr->width, ihdr->height);
    if (palettes)
    {
        png_set_PLTE(png_ptr, info_ptr, palettes, num_palette);
        printf("set PLTE %d\n", num_palette);
    }
    if (phys)
    {
        png_set_pHYs(png_ptr, info_ptr, phys->res_x, phys->res_y, phys->type);
    }
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return 0;
}
