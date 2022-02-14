
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>

void clocktest(void);

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");
    clocktest();
    return 0;
}
