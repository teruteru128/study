
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

int main(int argc, char *argv[])
{
    FILE *in = fopen("wordlist.txt", "r");
    if(!in)
        return 1;
    int ret = 0;
    FILE *out = fopen("wordout.txt", "w");
    if(!out)
    {
        ret = 1;
        goto fail;
    }
    regex_t reg;
    char *pattern = "list";
    regmatch_t match[1];
    ret = regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE | REG_ICASE);
    if(ret != 0)
    {
        size_t len = regerror(ret, &reg, NULL, 0);
        char *buf = malloc(len);
        if(buf)
        {
            regerror(ret, &reg, buf, len);
            fprintf(stderr, "%s\n", buf);
            free(buf);
        }
        ret = 1;
        goto fail;
    }
    char line[BUFSIZ];
    while(fgets(line, BUFSIZ, in) != NULL)
    {
        ret = regexec(&reg, line, 1, match, 0);
        if(ret == 0)
        {
            if(match[0].rm_so != -1)
            {
                strncpy(&line[match[0].rm_so], "list", 4);
            }
        }
        else
        {
            size_t len = regerror(ret, &reg, NULL, 0);
            char *buf = malloc(len);
            if(buf)
            {
                regerror(ret, &reg, buf, len);
                fprintf(stderr, "%s\n", buf);
                free(buf);
            }
        }
        fprintf(out, "%s", line);
        match[0].rm_so = 0;
        match[0].rm_eo = 0;
    }
fail:
    fclose(in);
    fclose(out);
    regfree(&reg);
    return ret;
}
