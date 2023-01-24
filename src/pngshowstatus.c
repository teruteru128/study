
#include "pngheaders.h"
#include <inttypes.h>
#include <limits.h>
#include <png.h>

int main(int argc, char const *argv[])
{
    struct IHDR in_ihdr1;
    struct IHDR in_ihdr2;
    char path[PATH_MAX];
    png_byte **a = NULL;
    png_byte **b = NULL;
    for (size_t i = 1; i < 62; i += 2)
    {
        snprintf(path, PATH_MAX,
                 "/mnt/g/iandm/image/shonenjumpplus.com/13933686331749163174/"
                 "page%02lu.png",
                 i);
        read_png(path, &in_ihdr1, NULL, NULL, NULL, NULL);
        printf("%s: image header: %" PRId32 " x %" PRId32 " ", path, in_ihdr1.width,
               in_ihdr1.height);
        printf("%d %d %d %d %d\n", in_ihdr1.bit_depth, in_ihdr1.color_type,
               in_ihdr1.interlace_method, in_ihdr1.compression_method,
               in_ihdr1.filter_method);
    }
    return 0;
}
