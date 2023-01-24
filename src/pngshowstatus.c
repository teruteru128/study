
#include "pngheaders.h"
#include <inttypes.h>
#include <limits.h>
#include <png.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    struct IHDR in_ihdr1;
    struct pHYs p;
    char path[PATH_MAX];
    struct stat st;
    int result = 0;
    for (size_t i = 1; i < argc; i++)
    {
        snprintf(path, PATH_MAX, argv[i], i);
        result = stat(argv[i], &st);
        if (result != 0)
        {
            perror("file/directory not found. continue.");
            continue;
        }
        if ((st.st_mode & S_IFMT) == S_IFDIR)
        {
            fprintf(stderr, "%s is dir\n", argv[i]);
            continue;
        }
        read_png(argv[i], &in_ihdr1, &p, NULL, NULL, NULL);
        printf("%s: image header: %" PRId32 " x %" PRId32 ", ", path,
               in_ihdr1.width, in_ihdr1.height);
        printf("bit depth:%d, color type: %d, interlace method: %d, "
               "compression method: %d, filter method: %d, ",
               in_ihdr1.bit_depth, in_ihdr1.color_type,
               in_ihdr1.interlace_method, in_ihdr1.compression_method,
               in_ihdr1.filter_method);
        printf("res x: %d, res y: %d, res type: %d\n", p.res_x, p.res_y,
               p.type);
    }
    return 0;
}
