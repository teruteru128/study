
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define SRC                                                                        \
    "551 5 ABCDEFG:2005/03/27 "                                                    \
    "12-34-56:12時34分頃,3,1,4,紀伊半島沖,ごく浅く,3.2,1,N12.3,E45.6," \
    "仙台管区気象台:-奈良県,+2,*下北山村,+1,*十津川村,*奈良川上村\r\n"

int printregerror(int r, const regex_t *reg)
{
    if (r != 0)
    {
        size_t errbuf_size = regerror(r, reg, NULL, 0);
        char *errbuf = malloc(errbuf_size);
        regerror(r, reg, errbuf, errbuf_size);
        fprintf(stderr, "%s\n", errbuf);
        free(errbuf);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int epsptest(void)
{

    int code;
    int hop;
    char format[BUFSIZ];
    snprintf(format, BUFSIZ, "%%d %%d %%%ds", BUFSIZ - 1);
    char buf[BUFSIZ];
    // sscanfは空白文字を読めないので不可
    sscanf(SRC, format, &code, &hop, buf);
    printf("%d, %d, %s\n", code, hop, buf);
    regex_t reg;
    /**
     * @brief Construct a new regex object
     * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
     * "^[[:digit:]]{3} [[:digit:]]+( .+)?$"
     * "^[[:digit:]]{3}( [[:digit:]]+( .*)?)?$"
     * REG_NEWLINEはCRを除外しないのでstrpbrkあたりで引っ掛けて除去が必要
     */
    int r = regcomp(&reg, "^([[:digit:]]{3}) ([[:digit:]]+)( (.+))?$",
                    REG_EXTENDED | REG_NEWLINE);
    printregerror(r, &reg);
    regmatch_t match[5];
    regoff_t diff = 0;
    if (regexec(&reg, SRC, 5, match, 0) == 0)
    {
        printf("matched : %d, %d\n", match[4].rm_so, match[4].rm_eo);
        diff = match[4].rm_eo - match[4].rm_so;
        strncpy(buf, SRC + match[4].rm_so, (size_t)diff);
        char *crlfptr = strpbrk(buf, "\r\n");
        if (crlfptr != NULL)
        {
            fprintf(stdout, "found crlf, %02x\n", *crlfptr);
            *crlfptr = '\0';
        }
        printf("%s\n", buf);
    }
    regfree(&reg);
    r = regcomp(&reg, " ", REG_EXTENDED);
    printregerror(r, &reg);
    size_t i = 0;
    if (regexec(&reg, SRC, 4, match, 0) == 0)
    {
        for (i = 0; i < 4; i++)
        {
            printf("match[%zu] = %d, %d\n", i, match[i].rm_so, match[i].rm_eo);
        }
    }
    regfree(&reg);
    return EXIT_SUCCESS;
}

int readsample(void)
{
    size_t length = 0;
    char *buffer = NULL;
    char *tmp = NULL;
    char inbuf[BUFSIZ];
    int fd = open("", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    ssize_t r = 0;
    while (1)
    {
        r = read(fd, inbuf, BUFSIZ);
        if (r < 0)
        {
            return 1;
        }
        length += r;
        tmp = realloc(buffer, length + 1);
        if (tmp == NULL)
        {
            perror("realloc");
            exit(1);
        }
        buffer = tmp;
        memcpy(buffer + (length - r), inbuf, r);
        buffer[length] = 0;
        ssize_t start = length - r - 1;
        if (start < 0)
        {
            start = 0;
        }
        if ((tmp = strstr(buffer + start, "\r\n")) != NULL)
        {
            size_t i = tmp - buffer;
            char *line = malloc(i + 1);
            memcpy(line, buffer, i);
            line[i] = 0;

            // ここでコンテキストと行データをセットにして別スレッドへ転送

            // truncate
            // 結局newBufferLengthもlengthもnullバイト分の長さを含んでないのよね
            size_t newBufferLength = length - i - 2;
            memmove(buffer, buffer + i + 2, newBufferLength);
            buffer[newBufferLength] = 0;
            char *newBuffer = realloc(buffer, newBufferLength + 1);
            if (newBuffer == NULL)
            {
                perror("realloc(truncate)");
                exit(1);
            }
            buffer = newBuffer;
        }
    }
}
