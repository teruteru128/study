
#include <locale.h>

int rip(void);

int main(void)
{
    setlocale(LC_ALL, "");
    rip();
    return 0;
}
