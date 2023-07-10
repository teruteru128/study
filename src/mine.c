
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
int main(int argc, char **argv, const char **envp)
{
    if (argc < 2)
    {
        return 1;
    }
    char *pattern = strdup(argv[1]);
    if (pattern == NULL)
    {
        return 1;
    }
    regex_t p = { 0 };

    int ret = 0;
    if ((ret = regcomp(&p, pattern,
                       REG_EXTENDED | REG_ICASE | REG_NEWLINE | REG_NOSUB))
        != 0)
    {
        size_t errbuf_size = regerror(ret, &p, NULL, 0);
        char *errbuf = malloc(errbuf_size);
        regerror(ret, &p, errbuf, errbuf_size);
        printf("%d, %s\n", ret, errbuf);
        free(errbuf);
        return 1;
    }
    printf("%u %u %u %u %u %u %u\n", p.can_be_null, p.regs_allocated,
           p.fastmap_accurate, p.no_sub, p.not_bol, p.not_eol,
           p.newline_anchor);
    char buf[BUFSIZ] = "";
    while (fgets(buf, BUFSIZ, stdin) != NULL)
    {
        if (regexec(&p, buf, 0, NULL, 0) == 0)
        {
            printf("どかーん！\n");
        }
    }
    regfree(&p);
    free(pattern);
    return 0;
}
