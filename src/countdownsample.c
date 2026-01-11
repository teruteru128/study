
#include <time.h>

#include "countdown.h"

int main(int argc, char const *argv[])
{
    time_t now = time(NULL);
    now += 10;
    struct tm tm;
    localtime_r(&now, &tm);
    countdowns(&tm);
    return 0;
}
