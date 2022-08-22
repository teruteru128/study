
#define _GNU_SOURCE

#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int hiho(int argc, char **argv, const char **envp)
{
    struct __jmp_buf_tag *buf = malloc(sizeof(struct __jmp_buf_tag) * 16);

    switch (setjmp(buf))
    {
    case 0:
        longjmp(buf, 1);
        break;
    case 1:
        return 114;
    default:
        break;
    }
    free(buf);

    return 0;
}
