
#ifndef STRING_LIST_H
#define STRING_LIST_H (1)

typedef struct string_list string_list;
typedef struct string_list
{
    char *str;
    string_list *next;
} string_list;

string_list *string_list_add(string_list *, char *);
string_list *str_split(char *, char *);
size_t string_list_size(string_list *);
void string_list_free(string_list *);
string_list *string_list_get_last_node(string_list *);

#endif
