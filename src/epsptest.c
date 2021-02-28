
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#define SRC "551 5 ABCDEFG:2005/03/27 12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6,仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

int epsptest()
{

  int code;
  int hop;
  char format[BUFSIZ];
  snprintf(format, BUFSIZ, "%%d %%d %%%ds", BUFSIZ - 1);
  char buf[BUFSIZ];
  // sscanfは空白文字を読めないので不可
  sscanf(SRC, format, &code, &hop, buf);
  printf("%d, %d, %s\n", code, hop, buf);
  regex_t reg;
  /**
   * @brief Construct a new regex object
   * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
   * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
   * "^[[:digit:]]{3}( [[:digit:]]+( .*)?)?$"
   * REG_NEWLINEはCRを除外しない
   */
  int r = regcomp(&reg, "^([[:digit:]]{3}) ([[:digit:]]+)( (.+))?$", REG_EXTENDED | REG_NEWLINE);
  if (r != 0)
  {
    switch (r)
    {
    case REG_BADRPT:
      fprintf(stderr, "badrpt %d\n", r);
      break;
    default:
      fprintf(stderr, "other %d\n", r);
      break;
    }
    return EXIT_FAILURE;
  }
  regmatch_t match[5];
  if (regexec(&reg, SRC, 5, match, 0) == 0)
  {
    printf("matched : %d, %d\n", match[4].rm_so, match[4].rm_eo);
    regoff_t diff = match[4].rm_eo - match[4].rm_so;
    strncpy(buf, SRC + match[4].rm_so, (size_t)diff);
    char *crlfptr = strpbrk(buf, "\r\n");
    if (crlfptr != NULL)
    {
      *crlfptr = '\0';
    }
    printf("%s\n", buf);
  }
  regfree(&reg);
  return EXIT_SUCCESS;
}