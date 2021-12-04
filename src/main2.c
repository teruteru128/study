
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    time_t machine_time = 0;
    struct tm machine_tm = { 0 };
    char buf[BUFSIZ] = "";
    for (size_t i = 0; i < 2000; i++)
    {
        machine_time = time(NULL);
        localtime_r(&machine_time, &machine_tm);
        strftime(buf, BUFSIZ, "%Ex %EX", &machine_tm);
        fprintf(stdout, "%s\n", buf);
        buf[0] = '\0';
        sleep(1);
    }

    return 0;
}
