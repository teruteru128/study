
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define PATTERN "list"

int main(int argc, char *argv[])
{
    FILE *in = fopen("wordlist.txt", "r");
    if (!in)
        return 1;
    int ret = 0;
    FILE *out = fopen("wordout.txt", "w");
    if (!out)
    {
        ret = 1;
        goto fail;
    }
    regex_t regex;
    regmatch_t match;
    ret = regcomp(&regex, PATTERN, REG_EXTENDED | REG_NEWLINE | REG_ICASE);
    if (ret != 0)
    {
        size_t len = regerror(ret, &regex, NULL, 0);
        char *buf = malloc(len);
        if (buf)
        {
            regerror(ret, &regex, buf, len);
            fprintf(stderr, "%s\n", buf);
            free(buf);
        }
        else
        {
            perror("malloc");
        }
        ret = 1;
        goto fail;
    }
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, in) != NULL)
    {
        ret = regexec(&regex, line, 1, &match, 0);
        if (ret == 0 && match.rm_so != -1)
        {
            memcpy(&line[match.rm_so], PATTERN, 4);
        }
        else
        {
            size_t len = regerror(ret, &regex, NULL, 0);
            char *buf = malloc(len);
            if (buf)
            {
                regerror(ret, &regex, buf, len);
                fprintf(stderr, "%s\n", buf);
                free(buf);
            }
        }
        fprintf(out, "%s", line);
    }
fail:
    fclose(in);
    fclose(out);
    regfree(&regex);
    return ret;
}
