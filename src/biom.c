
#include <stdint.h>
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MAXNUM 1000

static uint64_t array[MAXNUM + 1][MAXNUM + 1];

uint64_t binom_v3(uint64_t n, uint64_t k)
{
    if (array[n][k] == 0)
    {
        k = MIN(k, n - k);
        uint64_t val = 0;
        if (k == 0)
        {
            val = 1;
        }
        else
        {
            val = binom_v3(n - 1, k - 1) * n / k;
        }
        array[n][k] = val;
        return val;
    }
    else
    {
        return array[n][k];
    }
}

uint64_t binomial(uint64_t n, uint64_t k) { return binom_v3(n, k); }
