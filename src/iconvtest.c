
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <iconv.h>
#include <string.h>

#define SRC "551 5 ABCDEFG:2005/03/27 12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6,仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

int main(argc, argv)
int argc; char **argv;
{
  printf("%lu\r\n", strlen(SRC));
  iconv_t cd = iconv_open("Shift-JIS", "UTF-8");
  if (cd == (iconv_t)-1)
  {
    perror("iconv_open");
    return EXIT_FAILURE;
  }
  size_t srclen = strlen(SRC);
  size_t destlen = srclen * 3 + 1;
  char *tmpsrc = SRC;
  char *destbuf = malloc(destlen);
  if(!destbuf){
    perror("malloc");
    return EXIT_FAILURE;
  }
  char *head = destbuf;
  size_t ret = iconv(cd, &tmpsrc, &srclen, &destbuf, &destlen);
  if (ret == (size_t)-1)
  {
    perror("iconv");
    return -1;
  }
  *destbuf = '\0';
  char *tmp = realloc(head, strlen(head) + 1);
  if(tmp != NULL)
  {
    head = tmp;
  }
  printf("%lu\r\n", strlen(head));
  printf("%s", head);
  free(head);
  int r = iconv_close(cd);
  if(r != 0)
  {
    perror("iconv_close");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
