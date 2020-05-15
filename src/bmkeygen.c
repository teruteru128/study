
#include <stdio.h>
#include <stdlib.h>
#include <openssl/ec.h>

int main(int argc, char *argv[])
{
    size_t curve_list_size = EC_get_builtin_curves(NULL, 114514);
    if (curve_list_size == 0)
    {
        return EXIT_FAILURE;
    }
    EC_builtin_curve *list = calloc(curve_list_size, sizeof(EC_builtin_curve));
    if (list == NULL)
    {
        return EXIT_FAILURE;
    }
    size_t success_flag = EC_get_builtin_curves(list, curve_list_size);
    if (success_flag == 0)
    {
        return EXIT_FAILURE;
    }
    size_t i = 0;
    char *name = null;
    for (; i < success_flag; i++)
    {
        name = OBJ_nid2sn(list[i].nid);
        fprintf(stdout, "%d, %s\n", list[i].nid, list[i].comment);
    }
    free(list);
    return EXIT_SUCCESS;
}
