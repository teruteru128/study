
#include <regex.h>
#include <stdio.h>
#include <time.h>

#define MATCH_NUM 2

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        return 1;
    }
    regex_t pattern;
    regmatch_t matches[MATCH_NUM] = { { 0 } };
    int r = 0;
    r = regcomp(&pattern, "(a+)+b", REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    if (r != 0)
    {
        perror("regcomp");
        return 1;
    }
    r = regexec(&pattern, argv[1], 1, matches, 0);
    if (r != 0)
    {
        perror("regexec");
    }
    for(int i = 0; i < MATCH_NUM && matches[i].rm_so != -1; i++){
        printf("%d, %d\n", matches[i].rm_so, matches[i].rm_eo);
    }
    printf("%d\n", r);
    regfree(&pattern);
    return 0;
}
