
#ifndef SERVER_H

#include <netdb.h>

int init_server(char *);
void *do_service(void *arg);
void close_server();

struct service_config
{
    char service[NI_MAXSERV];
    // PF_UNSPEC or PF_INET or PF_INET6
    int protocol_family;
};

struct service_arg
{
    char port[NI_MAXSERV];
};

struct serverobject{
    int serversocket;
};

extern volatile int running;

#endif
