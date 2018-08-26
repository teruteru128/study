
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const static char *ten = "0000000000000000"
                         "1111111111111111"
                         "2222222222222222"
                         "3333333333333333"
                         "4444444444444444"
                         "5555555555555555"
                         "6666666666666666"
                         "7777777777777777"
                         "8888888888888888"
                         "9999999999999999"
                         "AAAAAAAAAAAAAAAA"
                         "BBBBBBBBBBBBBBBB"
                         "CCCCCCCCCCCCCCCC"
                         "DDDDDDDDDDDDDDDD"
                         "EEEEEEEEEEEEEEEE"
                         "FFFFFFFFFFFFFFFF";
const static char *one = "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF"
                         "0123456789ABCDEF";
static size_t parseHex(char **out, char *str)
{
    size_t length = strlen(str) / 2;
    size_t i = 0;
    unsigned int x;
    char *data = calloc(length, sizeof(char));
    static char table[256] = {0};
    if (table[0x30] == 0)
    {
        for (i = 0; i < 10; i++)
        {
            table[(0x30 + i)] = (char)i;
        }
        for (i = 0; i <= 6; i++)
        {
            table[(0x40 + i)] = (char)(9 + i);
        }
        for (i = 0; i <= 6; i++)
        {
            table[(0x60 + i)] = (char)(9 + i);
        }
    }
    if (!data)
    {
        perror(NULL);
        exit(1);
    }
    for (i = 0; i < length; i++)
    {
        data[i] = table[str[2 * i]]*16 | table[str[2 * i + 1]];
    }
    *out = data;
    return length;
}
static size_t parseHexArray(char **out, char **data)
{
    char *string = NULL;
    char **tmp = data;
    size_t length = 1;
    while (*tmp != NULL)
    {
        length += strlen(*tmp);
        tmp++;
    }
    string = calloc(length, sizeof(char));
    if (!string)
    {
        perror(NULL);
        exit(1);
    }
    tmp = data;
    while (*tmp != NULL)
    {
        string = strcat(string, *tmp);
        tmp++;
    }
    return parseHex(out, string);
}
int main(int argc, char **argv)
{
    char *filename = "FF.lzh";
    FILE *file = NULL;
    char *content[33] = {
        "4E002D6C68302DDE000000DE0000003E", // 00
        "06143F2002A0064D0F000189F090E04C", // 01
        "4953542E54585405004021001B004160", // 02
        "AFBDA4B04AC30100F3B9CFD74AC30100", // 03
        "D877423D4DC3010500007B3100004261", // 04
        "73653634204D616362696E6172792042", // 05
        "6D702049636F2043757220414E442D6D", // 06
        "61736B2054696666207061636B626974", // 07
        "7320434349545433464158656E636F64", // 08
        "6520504E47204352433136207A6C6962", // 09
        "204A504547205465582D647669204558", // 10
        "4520434F4D2056584420504946207461", // 11
        "72206E6864206E66642069736F393636", // 12
        "30204F4C452D636F6D706C657820677A", // 13
        "69702041646C65723332207A69702043", // 14
        "52433332204D532D636F6D7072657373", // 15
        "2043414220574146206175205374616E", // 16
        "646172644D494449206D7033204C5A48", // 17
        "207575656E636F64650D0A1A00",       // 18
        NULL};
    file = fopen(filename, "wb");
    if (!file)
    {
        perror(NULL);
        return 1;
    }
    char *data;
    size_t writelen = parseHexArray(&data, content);
    size_t i = 0;
    for (i = 0; i < writelen; i++)
    {
        printf("%02x", *(data + i)&0xff);
        if (i % 16 == 15)
        {
            printf("\n");
        }
    }
    fwrite(data, writelen, sizeof(char), file);
    fclose(file);
    return 0;
}
