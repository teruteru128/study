
#ifndef SERVER_H

int init_server(int *, char *);
void *do_service(void *arg);
void close_server();
struct service_arg
{
    int listen_socket;
};
#endif
