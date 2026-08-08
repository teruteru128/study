
#define _GNU_SOURCE
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int main(int argc, char **argv, const char **envp) {
  if (argc < 2) {
    return 1;
  }
  char *pattern = strdup(argv[1]);
  if (pattern == NULL) {
    return 1;
  }
  regex_t regex = {0};

  int ret = 0;
  const int flags = REG_EXTENDED | REG_ICASE | REG_NEWLINE | REG_NOSUB;
  if ((ret = regcomp(&regex, pattern, flags)) != 0) {
    size_t errbuf_size = regerror(ret, &regex, NULL, 0);
    char *errbuf = malloc(errbuf_size);
    regerror(ret, &regex, errbuf, errbuf_size);
    printf("%d, %s\n", ret, errbuf);
    free(errbuf);
    return 1;
  }
  printf("can_be_null: %u, regs_allocated: %u, fastmap_accurate: %u, no_sub: "
         "%u, not_bot: %u, not_eol: %u, newline_anchor: %u\n",
         regex.can_be_null, regex.regs_allocated, regex.fastmap_accurate,
         regex.no_sub, regex.not_bol, regex.not_eol, regex.newline_anchor);
  char buf[BUFSIZ] = "";
  while (fgets(buf, BUFSIZ, stdin) != NULL) {
    if (regexec(&regex, buf, 0, NULL, 0) == 0) {
      printf("マッチしますた！\n");
    }
  }
  regfree(&regex);
  free(pattern);
  return 0;
}
