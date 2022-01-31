
#ifndef SERVER_H

#include <netdb.h>

int init_server(char *);
void *do_service(void *arg);
void close_server();
struct service_arg
{
    char port[NI_MAXSERV];
};

extern volatile int shutdown;

#endif
