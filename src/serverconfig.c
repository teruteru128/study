
#define _GNU_SOURCE
#include "serverconfig.h"
#include <netdb.h>
#include <pthread.h>
#include <string.h>

struct config
{
    char host[NI_MAXHOST];
    char pad1[7];
    char service[NI_MAXSERV];
} config = { 0 };

pthread_once_t init_args_obj_once = PTHREAD_ONCE_INIT;
void init_args_obj() {
    // ここでconfigにデフォルト値を設定する
}

void parse_args(int argc, char *argv[])
{
    pthread_once(&init_args_obj_once, init_args_obj);
    return;
}
