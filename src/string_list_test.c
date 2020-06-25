
#include <stdio.h>
#include <string_list.h>

int main(int argc, char *argv[])
{
    string_list *list = string_list_add(NULL, "aaa");
    list = string_list_add(list, "bbb");
    list = string_list_add(list, "ccc");
    string_list *tmp = list;
    while (tmp->next != NULL)
    {
        printf("%s\n", tmp->str);
        tmp = tmp->next;
    }
    printf("%ld\n", string_list_size(list));
    printf("%s\n", tmp->str);
    string_list_free(list);
    return 0;
}
