
#include <stdio.h>
#include <stdlib.h>
#include <args_parse.h>

void init(void)
{
    return;
}

int main(int argc, const char *argv[])
{
    args_t *args = args_new();
    init();
    parse_args(args, argc, argv);
    return EXIT_SUCCESS;
}
