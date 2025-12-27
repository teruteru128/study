
#define _GNU_SOURCE
// #include <base64.h>
#include <bm.h>
// #include <bmapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
// #include <jsonrpc-glib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/random.h>
#include <fcntl.h>
#include <inttypes.h>
#include <time.h>
#include <bm_protocol.h>
#include <bm_network.h>

#define NAME "/TrBmTestClient:" BM_VERSION "/"
#define MAX_EVENTS 16

/*
network addressのエンコード
time:uint64_t
stream:uint32_t
services:uint64_t
IPv6/4 address:16byte
port:uint16_t
*/

// 128kb
#define INIT_CONNECT_BUFFER_SIZE 131072

/**
 * サブコマンド方式でできねえかな
 * addrgen
 *   新規アドレス生成
 *     乱数ソース
 *     既存鍵ソース
 *     パスフレーズ
 *       バージョン3
 *       バージョン4
 * sendmsg [fromaddress] [toaddress] [subject] [body]
 *   メッセージ送信
 *     PoWサーバー指定機能
 * bcast [fromaddress] [subject] [body] <moreoptions>
 *  配信
 */
int main(const int argc, const char **argv)
{
    // シンプルなBitMessageクライアントのサンプルコード
    epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }
    struct addrinfo hints, *res = NULL, *ptr = NULL;
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int rc = getaddrinfo("192.168.12.9", "8444", &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return EXIT_FAILURE;
    }

    ssize_t r = 0;
    int ret = 0;
    int sock = -1;
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock != -1)
            break;
    }
    if (connect(sock, ptr->ai_addr, ptr->ai_addrlen) == -1)
    {
        perror("connect");
        close(sock);
        close(epfd);
        return EXIT_FAILURE;
    }
    // non-blockingに設定
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    freeaddrinfo(res);
    struct sockaddr_storage local_addr, peer_addr;
    socklen_t local_len = sizeof(local_addr);
    socklen_t peer_len = sizeof(peer_addr);
    // ローカルアドレスの取得
    if (getsockname(sock, (struct sockaddr *)&local_addr, &local_len) == -1)
    {
        perror("getsockname error");
    }
    // ピアアドレス（相手先）の取得
    if (getpeername(sock, (struct sockaddr *)&peer_addr, &peer_len) == -1)
    {
        perror("getpeername error (may be normal for unconnected sockets)");
    }
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN | EPOLLET;
    // fdだけで足りるならfdだけでいいし足りなければポインタ使えばいい
    // ev.data.fd = sock;
    struct fd_data *data = new_fd_data(CLIENT_SOCKET, sock);
    // ev.data.ptr = malloc(sizeof(struct fd_data));
    if (data == NULL)
    {
        perror("malloc struct fd_data");
        exit(EXIT_FAILURE);
    }
    ev.data.ptr = data;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
    // version messageを送信
    int version = 3;
    postVersion(sock, NAME, version, &peer_addr, &local_addr);
    // epoll_waitも別スレッドで行い、メインスレッドではコネクション数管理を行いたい
    // (アウトバウンド16コネクション、インバウンド16コネクションとか。実際はインバウンド数に上限はつけたくないけど)
    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            struct fd_data *data = (struct fd_data *)events[i].data.ptr;
            if (data->type == CLIENT_SOCKET)
            {
                // Handle socket data
                char buffer[131072];
                while (1)
                {
                    errno = 0;
                    ssize_t bytes_read = read(data->fd, buffer, 131072);
                    if (bytes_read == -1)
                    {
                        int errno_saved = errno;
                        if (errno_saved == EAGAIN || errno_saved == EWOULDBLOCK)
                        {
                            // No more data to read
                            break;
                        }
                        perror("read");
                        break;
                    }
                    if (bytes_read == 0)
                    {
                        // Connection closed
                        fprintf(stderr, "Connection closed by server\n");
                        close(data->fd);
                        free_fd_data(data);
                        break;
                    }
                    // Append data to connectedBuffer
                    if (data->length + bytes_read > data->size)
                    {
                        data->size *= 2;
                        data->connectedBuffer = realloc(data->connectedBuffer, data->size);
                        if (data->connectedBuffer == NULL)
                        {
                            perror("realloc fail");
                            close(data->fd);
                            free_fd_data(data);
                            exit(EXIT_FAILURE);
                        }
                    }
                    memcpy(data->connectedBuffer + data->length, buffer, bytes_read);
                    data->length += bytes_read;
                    // fprintf(stderr, "Read %zd bytes, total length: %zu bytes\n", bytes_read, length);
                }
                while (data->length > 0)
                {
                    if (data->length < 24)
                    {
                        break;
                    }
                    // 読み込み済みデータが24バイト未満でも考慮済みなので安全！
                    struct message *msg = parse_message(data->connectedBuffer, data->length);
                    if (msg == NULL)
                    {
                        // fprintf(stderr, "Received command: %s\n", msg->command);
                        int net_payload_length = *((int *)(data->connectedBuffer + 16));
                        int payload_length = ntohl(net_payload_length);
                        // fprintf(stderr, "Payload length: %d\n", payload_length);
                        if (data->length < payload_length)
                        {
                            // ペイロード全体がまだ来ていない場合は待つ
                            // fprintf(stderr, "Incomplete message, waiting for more data(current length: %zu bytes)\n", length);
                            break;
                        }
                        int checksum = *((int *)(data->connectedBuffer + 20));
                        checksum = ntohl(checksum);
                        // checksumを検証する
                        // checksumはpayloadの最初の4バイトをSHA512でハッシュ化したものの最初の4バイト
                        unsigned char computed_checksum[64];
                        SHA512((data->connectedBuffer + 24), payload_length, computed_checksum);
                        int computed_checksum_int = *((int *)computed_checksum);
                        if (checksum != ntohl(computed_checksum_int))
                        {
                            fprintf(stderr, "Checksum mismatch! Expected: %08x, Computed: %08x\n", checksum, ntohl(computed_checksum_int));
                            // 不正なメッセージなので破棄
                            // 次のメッセージに備えてバッファを調整
                            memmove(data->connectedBuffer, data->connectedBuffer + 24 + payload_length, data->length - (24 + payload_length));
                            data->length -= (24 + payload_length);
                            continue;
                        }
                        // 不完全なメッセージ、または不正なメッセージ
                        fprintf(stderr, "Incomplete or invalid message, waiting for more data\n");
                        void *nextpackethead = memmem(data->connectedBuffer + 4, data->length - 4, magicbytes, 4);
                        if (nextpackethead != NULL)
                        {
                            // 次のメッセージの先頭が見つかった場合、その位置までスキップ
                            size_t skip_bytes = (unsigned char *)nextpackethead - data->connectedBuffer;
                            fprintf(stderr, "Skipping %zu bytes to next potential message\n", skip_bytes);
                            memmove(data->connectedBuffer, nextpackethead, data->length - skip_bytes);
                            data->length -= skip_bytes;
                        }
                        break;
                    }
                    // fprintf(stderr, "バッファに残ったペイロードデータの長さ: %zu, ", length - (24 + payload_length));
                    // fprintf(stderr, "Processed command: %s, payload length: %d\n", command, payload_length);
                    // 次のメッセージに備えてバッファを調整
                    memmove(data->connectedBuffer, data->connectedBuffer + 24 + msg->length, data->length - (24 + msg->length));
                    data->length -= (24 + msg->length);
                    process_command(data, msg);
                    // コマンド処理を全部別スレッドに委託してしまえばこのスレッドでfree_messageを呼び出す必要もなくなる
                    // push_command_process_queue(data, msg);
                    // 処理が完了
                    // メッセージ解放
                    // 別スレッドにオブジェクトを転送する場合はmsgごと転送し解放は転送先で行う。msgにはNULLをセットしておく。
                    free_message(msg);
                }
                // 再度読み込みが発生することがあるのでバッファをリセットせずにループに戻る
            }
            // 次のファイルディスクリプタへ
        }
    }
    return EXIT_SUCCESS;
}
