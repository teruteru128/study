
#include "pngheaders.h"
#include <math.h>
#include <stdlib.h>

/**
 * @brief pngファイルを読み込んで解像度を書き換えてファイルに書き出す
 *
 */
int main(int argc, char const *argv[], const char **envp)
{
    struct IHDR ihdr = { 0 };
    struct pHYs phys = { 0 };
    const char inpath[]
        = "/mnt/g/iandm/image/waifu2x/pixiv.net/"
          "87422440_p0(UpRGB)(noise_scale)(Level0)(x4.000000).png";
    const char outpath[]
        = "/mnt/g/iandm/image/waifu2x/pixiv.net/87422440_p0_350dpi.png";
    png_byte **row_pointers = NULL;
    read_png(inpath, &ihdr, &phys, NULL, NULL, &row_pointers);
    printf("read ok\n");
    // 350 dpi to dots per meter
    phys.res_x = phys.res_y = floor((350 * 10000) / 254.);
    write_png(outpath, &ihdr, &phys, NULL, NULL, row_pointers);
    for (size_t y = 0; y < ihdr.height; y++)
    {
        free(row_pointers[y]);
        row_pointers[y] = NULL;
    }
    free(row_pointers);
    return 0;
}
