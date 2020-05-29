
#include <stdlib.h>
#include "safe.h"

void safe_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}
