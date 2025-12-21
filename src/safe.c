
#include <stdlib.h>
#include "safe.h"

// free関数はNULLでも安全だからこの関数いらないと思うんですけど
void safe_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}
