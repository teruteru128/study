
#define _GNU_SOURCE
#include "config.h"

#include "yattaze.h"
#include <inttypes.h>
#include <math.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/types.h>
#endif

int hmac(const char *crypto, unsigned char *key, size_t keysiz,
         unsigned char *msg, size_t msglen, unsigned char *hash, size_t *s)
{
    // TODO: EVP_MAC が OpenSSL に直接導入されるのは ver. 3.0 以降
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", "provider=default");
    EVP_MAC_CTX *macctx = EVP_MAC_CTX_new(mac);
    OSSL_PARAM params[2]
        = { OSSL_PARAM_utf8_string(OSSL_MAC_PARAM_DIGEST, "SHA-1", 0),
            OSSL_PARAM_END };
    EVP_MAC_init(macctx, key, keysiz, params);
    EVP_MAC_update(macctx, msg, msglen);
    EVP_MAC_final(macctx, hash, s, 20);
    EVP_MAC_CTX_free(macctx);
#else
    // OpenSSL_add_all_algorithms();
    // OpenSSL_add_all_ciphers();
    // OpenSSL_add_all_digests();
    const EVP_MD *md = EVP_get_digestbyname("SHA-1");
    EVP_PKEY *pkey
        = EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key, (int)keysiz);
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(mdctx, NULL, md, NULL, pkey);
    EVP_DigestSignUpdate(mdctx, msg, msglen);
    EVP_DigestSignFinal(mdctx, hash, s);
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
#endif
    return 0;
}

static const int DIGITS_POWER[9]
    //  0  1   2    3     4      5       6        7         8
    = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

char *generateTOTP(unsigned char *key, size_t keysiz, time_t time,
                   int returnDigits, const char *crypto)
{
    uint64_t msgword = (uint64_t)time;
    unsigned char *msg = (unsigned char *)&msgword;

    unsigned char hash[EVP_MAX_MD_SIZE] = "";
    size_t hashsiz = EVP_MAX_MD_SIZE;
    if (hmac(crypto, key, keysiz, msg, 8, hash, &hashsiz) != 0)
    {
        return NULL;
    }

    int offset = hash[hashsiz - 1] & 0x0f;

    int binary
        = ((hash[offset] & 0x7f) << 24) | ((hash[offset + 1] & 0xff) << 16)
          | ((hash[offset + 2] & 0xff) << 8) | (hash[offset + 3] & 0xff);

    int otp = binary % DIGITS_POWER[returnDigits];

    // テキスト整形
    char format[16] = "";
    snprintf(format, 16, "%%0%dd", returnDigits);
    char *result = malloc(returnDigits + 1);
    // 念のためゼロクリア
    memset(result, 0, returnDigits + 1);
    // OTP文字列生成
    snprintf(result, returnDigits + 1, format, otp);

    return result;
}

void e(png_struct *a, png_const_charp b)
{
    png_get_error_ptr(a);
    printf("%s\n", b);
}

int hiho(int argc, char **argv, const char **envp)
{
    int32_t width = 0, height = 0, res_x = 0, res_y = 0;
    int bit_depth = 0, color_type = 0, interlace_method = 0,
        compression_method = 0, filter_method = 0;
    int type = PNG_RESOLUTION_METER;
    png_byte **row_pointers = NULL;
    {
        FILE *infp = fopen("/mnt/g/iandm/image/waifu2x/pixiv.net/"
                           "86287248_p0(UpRGB)(scale)(x4.000000).png",
                           "rb");
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
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL,
                                    (png_infopp)NULL);
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

        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                     &color_type, &interlace_method, &compression_method,
                     &filter_method);
        printf("%" PRId32 " %" PRId32 "\n", width, height);
        printf("%d %d %d %d %d\n", bit_depth, color_type, interlace_method,
               compression_method, filter_method);
        unsigned int ret
            = png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &type);
        printf("pHYs: %u %" PRId32 " %" PRId32 " %" PRId32 "\n", ret, res_x,
               res_y, type);

        // dupliacte rows
        png_byte **original_row_pointers = png_get_rows(png_ptr, info_ptr);
        row_pointers = malloc(sizeof(png_byte *) * height);
        size_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
        for (size_t y = 0; y < height; y++)
        {
            row_pointers[y] = malloc(rowsize);
            memcpy(row_pointers[y], original_row_pointers[y], rowsize);
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(infp);
    }
    printf("read ok\n");
    {
        FILE *outfp = fopen("86287248_p0_350dpi.png", "wb");
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
            perror("うんちー！2");
            /* If we get here, we had a problem reading the file. */
            return (EXIT_FAILURE);
        }
        png_init_io(outpng_ptr, outfp);
        /*
         * PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
         * PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
         */
        png_set_IHDR(outpng_ptr, outinfo_ptr, width, height, bit_depth,
                     color_type, interlace_method, compression_method,
                     filter_method);
        // 350 dpi to dots per meter
        res_x = res_y = ceil((350 * 10000) / 254.);
        png_set_pHYs(outpng_ptr, outinfo_ptr, res_x, res_y, type);
        png_set_rows(outpng_ptr, outinfo_ptr, row_pointers);
        png_write_png(outpng_ptr, outinfo_ptr, 0, NULL);
        png_destroy_write_struct(&outpng_ptr, &outinfo_ptr);
        fclose(outfp);
    }
    for (size_t y = 0; y < height; y++)
    {
        free(row_pointers[y]);
        row_pointers[y] = NULL;
    }
    free(row_pointers);

    return 0;
}
