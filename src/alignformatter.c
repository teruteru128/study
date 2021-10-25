
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    (void)argc, (void)argv;

    unsigned char publickey[72] = "";

    FILE *fin = fopen("publicKeys.bin", "rb");
    FILE *fout = fopen("publicKeys-aligned.bin", "wb");

    size_t items = 0;
    int ret = EXIT_SUCCESS;

    for (size_t i = 0; i < 0x4000000UL; i++)
    {
        items = fread(publickey, 65, 1, fin);
        if (items != 1)
        {
            ret = EXIT_FAILURE;
            perror("fread");
            break;
        }
        items = fwrite(publickey, 72, 1, fout);
        if (items != 1)
        {
            ret = EXIT_FAILURE;
            perror("fwrite");
            break;
        }
    }
    fclose(fin);
    fclose(fout);

    return ret;
}
