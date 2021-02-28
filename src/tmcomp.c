
#include <time.h>

int tmcomp(const struct tm *tm1, const struct tm *tm2)
{
  if (tm1 == NULL || tm2 == NULL)
    return 1;
  if (tm1->tm_sec == tm2->tm_sec && tm1->tm_min == tm2->tm_min && tm1->tm_hour == tm2->tm_hour && tm1->tm_mday == tm2->tm_mday && tm1->tm_mon == tm2->tm_mon && tm1->tm_year == tm2->tm_year)
    return 0;
  return -1;
}