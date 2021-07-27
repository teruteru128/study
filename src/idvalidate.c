
#include <stdio.h>
#include <math.h>
#include <string.h>

int check(char *id)
{
    int index, sum, num;
    for (sum = index = 0; index < 17; index++)
        sum += ((int)pow(2, 17 - index) % 11) * (id[index] - '0');

    num = (12 - (sum % 11)) % 11;
    if (num < 10)
        return (num == id[17] - '0');
    else
        return ('X' == id[17]);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || strlen(argv[1]) != 18)
        return 2;
    return !check(argv[1]);
}
