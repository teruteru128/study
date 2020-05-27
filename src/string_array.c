
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_array.h"

string_array *string_array_create(size_t initialCapacity)
{
}
int trimToSize(string_array *a)
{
}

static char *grow() {}
static char *growOne() {}
static int add_(char *a, char **e, size_t size, size_t length)
{
}
int string_array_add(string_array *data, char *a)
{
    add_(a, data->str, data->size, data->length);
    return 0;
}

string_array *string_array_split(const char *in, const char *delim)
{
    char *work = strdup(in);
    if(work == NULL){
        return NULL;
    }
    char **str = malloc(sizeof(char *) * 10);
    if(str == NULL){
        free(work);
        return NULL;
    }
    size_t length = 10;
    size_t size = 0;
    char *tp = NULL, *catch = NULL;
    for (tp = strtok_r(work, delim, &catch); tp; tp = strtok_r(NULL, delim, &catch), size++)
    {
        // string_array_add(data, tp);
        if (size > length)
        {
            char **tmp = realloc(str, sizeof(char *) * (length + 3));
            if (tmp == NULL)
            {
                free(work);
                free(str);
                return NULL;
            }
            str = tmp;
            length = length + 3;
        }
        str[size] = strdup(tp);
        if (str[size] == NULL)
        {
            free(work);
            for (size_t i = 0; i < size; i++)
            {
                free(str[i]);
            }
            free(str);
            return NULL;
        }
    }
    // int trimToSize(string_array *)
    // trimToSize(data);
    char *tmp = realloc(str, sizeof(char *) * (size > 0 ? size - 1 : size));
    if (tmp == NULL)
    {
        free(work);
        for (size_t i = 0; i < size; i++)
        {
            free(str[i]);
        }
        free(str);
        return NULL;
    }
    string_array *data = malloc(sizeof(string_array));
    if (data == NULL)
    {
        free(work);
        for (size_t i = 0; i < size; i++)
        {
            free(str[i]);
        }
        free(str);
        return NULL;
    }
    free(work);
    data->str = str;
    data->length = length;
    data->size = size;
    return data;
}
