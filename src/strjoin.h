
#ifndef STRJOIN_H
#define STRJOIN_H 1

#include <stddef.h>

struct stringjoiner_t
{
    char *prefix;
    char *delimiter;
    char *suffix;
    char **elements;
    size_t size;
    size_t length;
    char *emptyValue;
};

struct stringjoiner_t *stringjoiner_new(char *delimiter);
struct stringjoiner_t *stringjoiner_new2(char *delimiter, char *prefix, char *suffix);

int stringjoiner_set_empty_value(struct stringjoiner_t *joiner, char *emptyValue);

struct stringjoiner_t *stringjoiner_add(struct stringjoiner_t *joiner, char *newElement);
char *stringjoiner_join(struct stringjoiner_t *joiner);
size_t stringjoiner_length(struct stringjoiner_t *joiner);

// JavaのStringJoinderのようにデータ構造にまとめたほうがいいのでは……？
char *strjoin(char *delimiter, char **array, size_t arraylen);

#endif
