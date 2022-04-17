
#include <locale.h>

int rip(void);

int main(void)
{
    setlocale(LC_ALL, "");
    return rip();
}
