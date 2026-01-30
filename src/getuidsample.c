
#include <stdio.h>
#include <unistd.h>
int main(int argc, char *argv[], char *envp[]){
    uid_t uid = getuid();
    uid_t euid = geteuid();
    printf("%d, %d\n", uid, euid);
    return 0;
}
