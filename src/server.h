
#ifndef SERVER_H

int init_server(char *);
void *do_service(void *arg);
void close_server();
#endif
