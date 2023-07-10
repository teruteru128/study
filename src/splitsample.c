
#define _GNU_SOURCE 1
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "string-array.h"
#include "string-list.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 文字列の配列を返す
 * https://hg.openjdk.java.net/jdk/jdk12/file/06222165c35f/src/java.base/share/classes/java/lang/String.java#l2274
 */
int split(char **dest, const char separator, const char *input, size_t limit)
{
    size_t sepnum = 0;
    const size_t length = strlen(input) + 1;
    char *work = malloc(length);
    memcpy(work, input, length);
    char *catch = NULL;
    while ((catch = strchr(work, separator)) != NULL)
    {
        work = catch + 1;
        sepnum++;
    }
    free(work);

    return 0;
}

string_list *split_sl(char *src, char *regex, int limit) { return NULL; }

int split_regex_sa(string_array **dest, char *pattern, char *str, size_t limit)
{
    return 0;
}

int split_regex(char **dest, char *str, char *pattern, size_t limit)
{
    regex_t pt;
    int ret = regcomp(&pt, pattern, REG_EXTENDED | REG_NEWLINE | REG_ICASE);
    if (ret != 0)
    {
        return ret;
    }
    return 0;
}

int split_strtok(string_array **dest, char *delim, char *src, size_t limit)
{
    return 0;
}

/*
 * プロトコルフォーマットに合致しているか検査しないといけないので面倒くさい可能性が高い
 */
int split_by_strtok(const char *str)
{
    char *target = strdup(str);

    free(target);
    return EXIT_FAILURE;
}

void free_string_array(string_array *str) {}

int split_by_regex(char *str, char *regex) { return EXIT_SUCCESS; }
