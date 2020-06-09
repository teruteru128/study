//charset-convert.c

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "charset-convert.h"

static int convert(iconv_t cd, char **dest, const char *src)
{
  size_t srclen = strlen(src);
  size_t destlen = srclen * 3 + 1;
  char *destbuf = malloc(destlen);
  char *head = destbuf;
  size_t ret = iconv(cd, (char **)&src, &srclen, &destbuf, &destlen);
  if (ret == (size_t)-1)
  {
    perror("iconv");
    return -1;
  }
  *destbuf = '\0';
  *dest = realloc(head, strlen(head) + 1);
  return 0;
}

int encode_utf8_2_sjis(char **dest, const char *src)
{
  iconv_t cd = iconv_open("SHIFT_JIS", "UTF-8");
  if (cd == (iconv_t)-1)
  {
    perror("iconv_open");
    return -1;
  }
  int ret = convert(cd, dest, src);
  iconv_close(cd);
  return ret;
}

int encode_utf8_2_unicode(char **dest, const char *src)
{
  iconv_t cd = iconv_open("Unicode", "UTF-8");
  if (cd == (iconv_t)-1)
  {
    perror("iconv_open");
    return -1;
  }
  int ret = convert(cd, dest, src);
  iconv_close(cd);
  return ret;
}
