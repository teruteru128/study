
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*
   * 暗黒通信団 ファイルフォーマット小辞典
   */

#define TEN "0000000000000000" \
            "1111111111111111" \
            "2222222222222222" \
            "3333333333333333" \
            "4444444444444444" \
            "5555555555555555" \
            "6666666666666666" \
            "7777777777777777" \
            "8888888888888888" \
            "9999999999999999" \
            "AAAAAAAAAAAAAAAA" \
            "BBBBBBBBBBBBBBBB" \
            "CCCCCCCCCCCCCCCC" \
            "DDDDDDDDDDDDDDDD" \
            "EEEEEEEEEEEEEEEE" \
            "FFFFFFFFFFFFFFFF"

#define ONE "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF" \
            "0123456789ABCDEF"

/*
 * strを16進数文字列としてパースします。
 */
static size_t parseHex(char **out, const char *str)
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
    x = str[i << 1];
    data[i] = table[x] << 4;
    x = str[(i << 1) + 1];
    data[i] |= table[x];
  }
  *out = data;
  return length;
}

int main(int argc, char **argv)
{
  char *filename = "FF.lzh";
  FILE *file = NULL;
  char *content =
      "4E002D6C68302DDE000000DE0000003E" // 00
      "06143F2002A0064D0F000189F090E04C" // 01
      "4953542E54585405004021001B004160" // 02
      "AFBDA4B04AC30100F3B9CFD74AC30100" // 03
      "D877423D4DC3010500007B3100004261" // 04
      "73653634204D616362696E6172792042" // 05
      "6D702049636F2043757220414E442D6D" // 06
      "61736B2054696666207061636B626974" // 07
      "7320434349545433464158656E636F64" // 08
      "6520504E47204352433136207A6C6962" // 09
      "204A504547205465582D647669204558" // 10
      "4520434F4D2056584420504946207461" // 11
      "72206E6864206E66642069736F393636" // 12
      "30204F4C452D636F6D706C657820677A" // 13
      "69702041646C65723332207A69702043" // 14
      "52433332204D532D636F6D7072657373" // 15
      "2043414220574146206175205374616E" // 16
      "646172644D494449206D7033204C5A48" // 17
      "207575656E636F64650D0A1A00"       // 18
      ;
  char *data = NULL;
  size_t outlen = parseHex(&data, content);
  size_t i = 0;
  for (i = 0; i < outlen; i++)
  {
    printf("%02x", data[i] & 0xff);
    if (i % 16 == 15)
    {
      fputs("\n", stdout);
    }
  }
  file = fopen(filename, "wb");
  if (!file)
  {
    perror("fopen");
    return 1;
  }
  size_t fwb = fwrite(data, outlen, sizeof(char), file);
  if (fwb < 1)
  {
    fprintf(stderr, "Failed to write : %s\n", strerror(errno));
  }
  int ret = fclose(file);
  if (ret == EOF)
  {
    fprintf(stderr, "Failed to close : %s\n", strerror(errno));
  }
  return EXIT_SUCCESS;
}
