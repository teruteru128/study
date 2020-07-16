
#include <stdio.h>

size_t parseHex(unsigned char **out, const char *str);
char *base58encode(unsigned char *input, size_t length);

int main(int argc, char const *argv[])
{
    // 00000D9663F57318B4E52288BFDC8B3C23E84DE1
    char *hex = "000111d38e5fc9071ffcd20b4a763cc9ae4f252bb4e48fd66a835e252ada93ff480d6dd43dc62a641155a5";
    unsigned char *in = NULL;
    unsigned int len = (unsigned int)parseHex(&in, hex);
    printf("%u\n", len);
    //memset(in, 0, 10);
    char *out = base58encode(in, len);
    printf("%s\n", out);
    free(in);
    free(out);
    return 0;
}
