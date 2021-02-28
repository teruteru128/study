
#include <stdio.h>
#include <printint.h>
#include <limits.h>

void snprintUInt64test()
{
  char buf[BUFSIZ] = "7777777777";
  size_t len = snprintUInt64(buf, BUFSIZ, ULONG_MAX);
  printf("%zu, %s\n", len, buf);
}