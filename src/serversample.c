
#define _GNU_SOURCE 1
#include "queue.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

/**
 * @brief open listened socket
 * オプションとかは仮引数で
 *
 * @return int listened socket
 */
static int openlistensocket(struct sockaddr **addr, socklen_t *len)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = NULL;
    getaddrinfo(NULL, "8080", &hints, &res);
    int s = -1;
    for (struct addrinfo *ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        printaddrinfo(ptr);
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (s < 0)
        {
            s = -1;
            continue;
        }

        if (bind(s, ptr->ai_addr, ptr->ai_addrlen) != 0)
        {
            close(s);
            s = -1;
            continue;
        }

        if (listen(s, 16) != 0)
        {
            close(s);
            s = -1;
            continue;
        }
        if (addr != NULL && len != NULL)
        {
            *addr = malloc(ptr->ai_addrlen);
            *len = ptr->ai_addrlen;
            memcpy(*addr, ptr->ai_addr, *len);
        }
        break;
    }
    freeaddrinfo(res);
    return s;
}

struct e_arg
{
    int epoll_fd;
    int listen_socket;
};

struct sockinfo;

struct sockinfo
{
    int fd;
    struct sockaddr *addr;
    socklen_t len;
    struct sockinfo *next;
};

static volatile int running = 1;

QUEUE_DEFINE(downloading_queue);

static void downloading_put(struct sockinfo *info)
{
    put(&downloading_queue, info);
}

static struct sockinfo *downloading_take()
{
    return (struct sockinfo *)take(&downloading_queue);
}

/**
    if(listensocket == socket){
        register to epoll fd
    }else{
        send (push?/forward?) downloading thread
    }
 */
static void *receivethread(void *arg)
{
    struct e_arg *arg1 = (struct e_arg *)arg;
    int epoll_fd = arg1->epoll_fd;
    int listen_socket = arg1->listen_socket;
    free(arg1);
    struct epoll_event *events = malloc(sizeof(struct epoll_event) * 1024);
    int i;
    int c;
    int acceptsocket = -1;
    while (running)
    {
        c = epoll_wait(epoll_fd, events, 1024, 1000);
        if (c < 0)
        {
            perror("epoll_wait");
        }
        for (i = 0; i < c; i++)
        {
            if (((struct sockinfo *)events[i].data.ptr)->fd == listen_socket)
            {
                struct sockinfo *info = malloc(sizeof(struct sockinfo));
                info->addr = malloc(sizeof(struct sockaddr_storage));
                info->len = sizeof(struct sockaddr_storage);
                info->next = NULL;
                acceptsocket = accept(listen_socket, info->addr, &info->len);
                if (acceptsocket >= 0)
                {
                    // register to epoll fd
                    struct epoll_event event = { 0 };
                    event.events = EPOLLIN;
                    info->fd = acceptsocket;
                    event.data.ptr = info;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, acceptsocket, &event);
                }
                else
                {
                    free(info->addr);
                    free(info);
                }
            }
            else
            {
                downloading_put((struct sockinfo *)events[i].data.ptr);
            }
        }
    }

    return NULL;
}

static pthread_t launche(int epollfd, int listen_socket, pthread_t *thread)
{
    struct e_arg *arg1 = malloc(sizeof(struct e_arg));
    arg1->epoll_fd = epollfd;
    arg1->listen_socket = listen_socket;

    return pthread_create(thread, NULL, receivethread, arg1);
}
static void *downloadingthread(void *arg) { return NULL; }
static void *processdatathread(void *arg) { return NULL; }

int main(int argc, char const *argv[])
{
    struct sockinfo info = { 0 };
    int listensock = openlistensocket(&info.addr, &info.len);
    info.fd = listensock;
    printf("listen: %d\n", listensock);
    close(listensock);

    // create epoll fd
    int e = epoll_create1(0);
    if (e < 0)
    {
        return 1;
    }

    // register listen socket to epoll fd
    // この epoll_event
    // の構造体って中でポインタを保持してんのかねぇ？それともコピー？
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.events = EPOLLIN;
    event.data.ptr = &info;
    epoll_ctl(e, EPOLL_CTL_ADD, listensock, &event);

    // launch accept/signal receive thread
    pthread_t thread;
    launche(e, listensock, &thread);

    // launch downloading thread
    // launch processing data thread
    return 0;
}
