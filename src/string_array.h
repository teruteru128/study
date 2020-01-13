
#ifndef STRING_ARRAY_H
#define STRING_ARRAY_H

#include <stdio.h>

typedef struct string_array string_array;

typedef struct string_array{
  /* memory area */
  char** str;
  /* num of strings */
  size_t size;
  /* memory area length */
  size_t length;
} string_array;

/* If the initial capacity is less than or equal to 0, use 10 as the initial capacity.  */
string_array* string_array_create(size_t initialCapacity);
int string_array_add(string_array *, char *);
string_array *string_array_split(const char *in, const char *delim);

#endif
