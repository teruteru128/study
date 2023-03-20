
#define _GNU_SOURCE 1
#define _DEFAULT_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include "countdown.h"
#include "countdown2038.h"
#include "pngheaders.h"
#include "pngsample_gennoise.h"
#include "queue.h"
#include "randomsample.h"
#include "roulette.h"
#include "searchAddressFromExistingKeys.h"
#include "timeutil.h"
#include <CL/opencl.h>
#include <bm.h>
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <gmp.h>
#include <iconv.h>
#include <inttypes.h>
#include <java_random.h>
#include <jsonrpc-glib.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <omp.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <printaddrinfo.h>
#include <regex.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <jpeglib.h> // jpeglibはstdioより下(FILEが依存しているため)

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/provider.h>
#include <openssl/types.h>
#endif

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

/**
 * @brief
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * ↓getDoubleで可能な限り1に近い値が出るseed
 * 155239116123415
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 * ハッシュの各バイトを１バイトにORで集約して結果が0xffにならなかったら成功
 * 丸数字の1から50までforで出す
 * timer_create+sigeventでタイマーを使って呼ばれたスレッドから新しくスレッドを起動する
 * TODO: CでSSLエンジンを使う
 * TODO: CでLibSSLなSSLエンジンを使う
 * TODO: javaでも直接SSLEngineを使ってみる
 * TODO: SocketChannel + SSLEngine + Selector
 * TODO: bitmessageをCで実装する、bitmessaged + bmctl の形式が良い
 *
 * decodable random source?
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int entrypoint(int argc, char **argv, char *const *envp)
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
    // この epoll_event の構造体って中で保持してんのかねぇ？それともコピー？
    struct epoll_event event = { 0 };
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
