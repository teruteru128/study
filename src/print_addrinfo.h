
#ifndef PRINT_ADDRINFO_H
#define PRINT_ADDRINFO_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
void print_addrinfo(struct addrinfo *);

#endif
