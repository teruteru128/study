
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STR_HYPHEN ("-")
#define STR_DOUBLE_HYPHEN ("--")
#define STR_REGISTER ("register")
#define STR_LOGIN ("login")
#define STR_LOGOUT ("logout")
#define STR_INIT ("init")
#define STR_TWEET_LENGTH ("tweet-length")
#define STR_HELP ("help")
#define STR_L ("l")

typedef enum logicswitch_e
{
    TWEET = 0,
    APP_REGISTER,
    LOGIN,
    LOGOUT,
    INIT,
    TWEET_LENGTH,
    HELP,
} LogicSwtich;

/**
 * コマンドライン引数パーササンプル
 */
typedef struct args
{
    LogicSwtich logic;
    int hasCommandLineArguments;
    char *status;
    char **statuses_v;
    size_t statuses_c;
} Args;

static Args *parseCommandLineArgs(Args *args, int argc, char **argv)
{
    args->hasCommandLineArguments = true;
    return args;
}

static Args *parseStdin(Args *args)
{
    args->hasCommandLineArguments = false;
    return args;
}

/**
 * @brief 
 * --help
 * --verbose
 * --version
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
    Args args;
    if (argc < 2)
    {
        parseStdin(&args);
    }
    else
    {
        parseCommandLineArgs(&args, argc, argv);
    }
    // メインコマンドのhelpとサブコマンドのhelpを別に実装するには？
    int h = 0;
    int v = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            h = 1;
        }
        else if (strcmp(argv[i], "--version") == 0)
        {
            v = 1;
        }
    }
    printf("%d %d\n", h, v);
    return EXIT_SUCCESS;
}
