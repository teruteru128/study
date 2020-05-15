
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "charset-convert.h"

/*
  http://projectseven.jp/seven/7key.htm
  http://projectseven.jp/seven/tool.htm
  https://twitter.com/teruteru128/status/1177140350428708864
  SJISに変換する
  1バイト=8桁で2進数変換文字列に変換する
  1バイト単位でビットを逆順に並び替える
*/
int encode(char **out, const char *in)
{
  size_t len = strlen(in);
  char *work = malloc(len * 8 + 1);
  if (work == NULL)
  {
    perror("malloc");
    return -1;
  }
  memset(work, 0, len * 8 + 1);
  size_t i = 0, j = 0;
  for (i = 0; i < len; i++)
  {
    for (j = 0; j < 8; j++)
    {
      work[i * 8 + j] = '0' + ((in[i] >> j) & 0x01);
    }
  }
  work[len * 8] = 0;
  *out = work;
  return 0;
}

int main(int argc, char *argv[])
{
  char *out = NULL;
  if (argc < 2)
  {
    fputs("", stdout);
    return EXIT_SUCCESS;
  }
  char *in = argv[1];
  if (encode_utf8_2_sjis(&out, in) != 0)
  {
    return EXIT_FAILURE;
  }
  char *catch = NULL;
  int err = encode(&catch, out);
  fputs(catch, stdout);
  fputc('\n', stdout);
  free(out);
  free(catch);
  return EXIT_SUCCESS;
}
