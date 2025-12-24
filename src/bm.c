
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

#define NAME "/TRBMTESTCLIENT:" VERSION "/"
#define MAX_EVENTS 16

int epfd = -1;

static const unsigned char magicbytes[] = {0xe9, 0xbe, 0xb4, 0xd9};

struct message;

/*
network addressのエンコード
time:uint64_t
stream:uint32_t
services:uint64_t
IPv6/4 address:16byte
port:uint16_t
*/

static unsigned char *endodeVariableLengthListOfIntegers(uint64_t *list, size_t listlen, size_t *outlen)
{
    size_t total_len = 0;
    unsigned char *result = encodeVarint((uint64_t)listlen, &total_len);
    fprintf(stderr, "list len encoded to %zu bytes\n", total_len);
    for (size_t i = 0; i < listlen; i++)
    {
        size_t item_len = 0;
        unsigned char *item_encoded = encodeVarint(list[i], &item_len);
        fprintf(stderr, "item %zu encoded to %zu bytes\n", i, item_len);
        result = realloc(result, total_len + item_len);
        memcpy(result + total_len, item_encoded, item_len);
        total_len += item_len;
        free(item_encoded);
    }
    if (outlen != NULL)
    {
        *outlen = total_len;
    }
    return result;
}

static unsigned char *createVersionMessage(const char *user_agent_str, int version, struct sockaddr_storage *peer_addr, struct sockaddr_storage *local_addr, int sock, size_t *out_length)
{
    // user_agent
    size_t ua_len = 0;
    unsigned char *user_agent = encodeVarStr(user_agent_str, &ua_len);
    size_t payload_length = 82 + ua_len; // 固定長部分 + 可変長ユーザーエージェント
    unsigned char *payload = malloc(payload_length);
    size_t offset = 0;
    uint32_t net_version = htobe32((uint32_t)version);
    memcpy(payload + offset, &net_version, 4);
    offset += 4;
    uint64_t services = 0;
    uint64_t net_services = htobe64(services);
    memcpy(payload + offset, &net_services, 8);
    offset += 8;
    uint64_t timestamp = (uint64_t)time(NULL);
    uint64_t net_timestamp = htobe64(timestamp);
    memcpy(payload + offset, &net_timestamp, 8);
    offset += 8;
    // addr_recv
    encodeNetworkAddress(payload + offset, peer_addr);
    offset += 26;
    // addr_from
    encodeNetworkAddress(payload + offset, local_addr);
    offset += 26;
    uint64_t nonce = 0;
    getrandom(&nonce, sizeof(nonce), GRND_NONBLOCK);
    uint64_t net_nonce = htobe64(nonce);
    memcpy(payload + offset, &net_nonce, 8);
    offset += 8;
    // user_agent
    memcpy(payload + offset, user_agent, ua_len);
    offset += ua_len;
    // stream_numbers
    size_t stream_list_len = 1;
    size_t stream_list_encoded_len = 2;
    unsigned char stream_list_encoded[] = {1, 1};
    memcpy(payload + offset, stream_list_encoded, stream_list_encoded_len);
    offset += stream_list_encoded_len;
    free(user_agent);
    // 全体の長さをセット
    if (out_length != NULL)
    {
        *out_length = payload_length;
    }
    return payload;
}

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
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);
    // version messageを送信
    size_t versionmsglen = 0;
    int version = 3;
    unsigned char *versionmsg = createVersionMessage(NAME, version, &peer_addr, &local_addr, sock, &versionmsglen);
    // ヘッダの作成
    unsigned char header[24];
    memcpy(header, magicbytes, 4);
    char command[12] = {0};
    strncpy(command, "version", 12);
    memcpy(header + 4, command, 12);
    uint32_t net_payload_length = htobe32((uint32_t)versionmsglen);
    memcpy(header + 16, &net_payload_length, 4);
    // checksumの計算
    unsigned char checksum_full[64];
    SHA512(versionmsg, versionmsglen, checksum_full);
    unsigned char checksum[4];
    memcpy(checksum, checksum_full, 4);
    memcpy(header + 20, checksum, 4);
    // 送信
    ssize_t a = write(sock, header, 24);
    ssize_t b = write(sock, versionmsg, versionmsglen);
    if (a != 24 || b != (ssize_t)versionmsglen)
    {
        perror("write error");
        close(sock);
        close(epfd);
        return EXIT_FAILURE;
    }
    free(versionmsg);
    unsigned char *connectedBuffer = malloc(BUFSIZ);
    size_t size = BUFSIZ;
    size_t length = 0;
    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == sock)
            {
                // Handle socket data
                char buffer[BUFSIZ];
                while (1)
                {
                    errno = 0;
                    ssize_t bytes_read = read(sock, buffer, BUFSIZ);
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
                        close(sock);
                        break;
                    }
                    // Append data to connectedBuffer
                    if (length + bytes_read > size)
                    {
                        size *= 2;
                        connectedBuffer = realloc(connectedBuffer, size);
                        if (connectedBuffer == NULL)
                        {
                            perror("realloc");
                            close(sock);
                            return EXIT_FAILURE;
                        }
                    }
                    memcpy(connectedBuffer + length, buffer, bytes_read);
                    length += bytes_read;
                    // fprintf(stderr, "Read %zd bytes, total length: %zu bytes\n", bytes_read, length);
                }
                while (length > 0)
                {
                    char command[13] = {0};
                    strncpy(command, connectedBuffer + 4, 12);
                    command[12] = '\0';
                    // fprintf(stderr, "Received command: %s\n", command);
                    int payload_length = *((int *)(connectedBuffer + 16));
                    payload_length = ntohl(payload_length);
                    // fprintf(stderr, "Payload length: %d\n", payload_length);
                    if (length < payload_length)
                    {
                        // ペイロード全体がまだ来ていない場合は待つ
                        // fprintf(stderr, "Incomplete message, waiting for more data(current length: %zu bytes)\n", length);
                        break;
                    }
                    int checksum = *((int *)(connectedBuffer + 20));
                    checksum = ntohl(checksum);
                    // checksumを検証する
                    // checksumはpayloadの最初の4バイトをSHA512でハッシュ化したものの最初の4バイト
                    unsigned char computed_checksum[64];
                    SHA512((connectedBuffer + 24), payload_length, computed_checksum);
                    int computed_checksum_int = *((int *)computed_checksum);
                    if (checksum != ntohl(computed_checksum_int))
                    {
                        fprintf(stderr, "Checksum mismatch! Expected: %08x, Computed: %08x\n", checksum, ntohl(computed_checksum_int));
                        // 不正なメッセージなので破棄
                        // 次のメッセージに備えてバッファを調整
                        memmove(connectedBuffer, connectedBuffer + 24 + payload_length, length - (24 + payload_length));
                        length -= (24 + payload_length);
                        continue;
                    }
                    // コマンドに対する処理を Strategy パターンを模倣して実装
                    // ここに各コマンドに対する処理を追加
                    // もしcommandが"verack"なら
                    if (strcmp(command, "verack") == 0)
                    {
                        fprintf(stderr, "Received verack message\n");
                        // verackに対する処理をここに書く
                    }
                    else if (strcmp(command, "version") == 0)
                    {
                        fprintf(stderr, "Received version message\n");
                        unsigned char *payload = (connectedBuffer + 24);
                        uint32_t version = ntohl(*((uint32_t *)payload));
                        uint64_t services = be64toh(*((uint64_t *)(payload + 4)));
                        uint64_t timestamp = be64toh(*((uint64_t *)(payload + 12)));
                        fprintf(stderr, "Version: %u, Services: %" PRIu64 ", Timestamp: %" PRIu64 "\n", version, services, timestamp);
                        printNetworkAddress(payload + 20, 26); // addr_recv
                        printNetworkAddress(payload + 46, 26); // addr_from
                        uint64_t nonce = be64toh(*((uint64_t *)(payload + 72)));
                        fprintf(stderr, "Nonce: %016" PRIx64 "\n", nonce);
                        // user_agentのデコード
                        size_t offset = 80;
                        size_t outlen = 0;
                        // varstrのデコード
                        // 先頭のvarintで長さを取得
                        uint64_t ua_len = decodeVarint(payload + offset, &outlen);
                        offset += outlen;
                        char *user_agent = malloc(ua_len + 1);
                        memcpy(user_agent, payload + offset, ua_len);
                        user_agent[ua_len] = '\0';
                        offset += ua_len;
                        fprintf(stderr, "User Agent: %s\n", user_agent);
                        free(user_agent);
                        // stream_numbersのデコード
                        uint64_t stream_count = decodeVarint(payload + offset, &outlen);
                        offset += outlen;
                        fprintf(stderr, "Stream Count: %" PRIu64 "\n", stream_count);
                        fprintf(stderr, "Streams: ");
                        for (uint64_t i = 0; i < stream_count; i++)
                        {
                            uint64_t stream_num = decodeVarint(payload + offset, &outlen);
                            offset += outlen;
                            fprintf(stderr, "%" PRIu64 " ", stream_num);
                        }
                        fprintf(stderr, "\n");
                        // verackコマンドを送信する
                        unsigned char verack_header[24];
                        memcpy(verack_header, magicbytes, 4);
                        char verack_command[12] = {0};
                        strncpy(verack_command, "verack", 12);
                        memcpy(verack_header + 4, verack_command, 12);
                        uint32_t verack_payload_length = 0;
                        uint32_t net_verack_payload_length = htobe32(verack_payload_length);
                        memcpy(verack_header + 16, &net_verack_payload_length, 4);
                        unsigned char verack_checksum[4] = {0xcf, 0x83, 0xe1, 0x35}; // verackのペイロードは空なのでSHA512("")の最初の4バイト
                        memcpy(verack_header + 20, verack_checksum, 4);
                        ssize_t w = write(sock, verack_header, 24);
                        if (w != 24)
                        {
                            perror("write verack error");
                            close(sock);
                            break;
                        }
                    }
                    else if (strcmp(command, "addr") == 0)
                    {
                        fprintf(stderr, "Received addr message\n");
                        unsigned char *payload = (connectedBuffer + 24);
                        // decode varint number of addresses
                        size_t offset = 0;
                        size_t outlen = 0;
                        uint64_t addr_count = 0;
                        addr_count = decodeVarint(payload + offset, &outlen);
                        offset += outlen;
                        fprintf(stderr, "Number of addresses: %" PRIu64 "\n", addr_count);
                        // 各アドレスをデコード
                        struct tm tm_info;
                        char time_buffer[128];
                        uint64_t addr_count2 = (payload_length - offset) / 38;
                        for (uint64_t i = 0; i < addr_count2; i++)
                        {
                            uint64_t time = be64toh(*((uint64_t *)(payload + offset)));
                            offset += 8;
                            uint32_t stream = ntohl(*((uint32_t *)(payload + offset)));
                            offset += 4;
                            uint64_t services = be64toh(*((uint64_t *)(payload + offset)));
                            offset += 8;
                            unsigned char ip[16];
                            memcpy(ip, payload + offset, 16);
                            offset += 16;
                            uint16_t port = ntohs(*((uint16_t *)(payload + offset)));
                            offset += 2;
                            localtime_r((time_t *)&time, &tm_info);
                            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
                            fprintf(stderr, "  Address %" PRIu64 ": time=%" PRIu64 "(%s), stream=%" PRIu32 ", services=%016" PRIx64 ", port=%u,", i, time, time_buffer, stream, services, port);
                            // IPアドレスの表示
                            if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0 &&
                                ip[4] == 0 && ip[5] == 0 && ip[6] == 0 && ip[7] == 0 &&
                                ip[8] == 0 && ip[9] == 0 && ip[10] == 0xff && ip[11] == 0xff)
                            {
                                // IPv4-mapped IPv6 address
                                fprintf(stderr, " IPv4 Address: %u.%u.%u.%u\n", ip[12], ip[13], ip[14], ip[15]);
                            }
                            else
                            {
                                // IPv6 address
                                char ipv6_str[40];
                                snprintf(ipv6_str, sizeof(ipv6_str),
                                         "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                                         "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                                         ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7],
                                         ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
                                fprintf(stderr, " IPv6 Address: %s\n", ipv6_str);
                            }
                        }
                    }
                    else if (strcmp(command, "inv") == 0)
                    {
                        fprintf(stderr, "Received inv message\n");
                        unsigned char *payload = (connectedBuffer + 24);
                        size_t offset = 0;
                        size_t outlen = 0;
                        uint64_t inv_count = 0;
                        inv_count = decodeVarint(payload + offset, &outlen);
                        offset += outlen;
                        // inv_countはリモートが保持しているオブジェクト数でペイロードに含まれるオブジェクト数とは限らない
                        uint64_t inv_count2 = (payload_length - offset) / 32;
                        fprintf(stderr, "Declared inv count: %" PRIu64 ", Actual inv count in payload: %" PRIu64 "\n", inv_count, inv_count2);
                        if (inv_count != inv_count2)
                        {
                            fprintf(stderr, "Warning: inv count mismatch! Declared: %" PRIu64 ", Actual: %" PRIu64 "\n", inv_count, inv_count2);
                        }
                        // for (uint64_t i = 0; i < inv_count2; i++)
                        // {
                        //     unsigned char object_hash[32];
                        //     memcpy(object_hash, payload + offset, 32);
                        //     offset += 32;
                        //     fprintf(stderr, "  Inventory Item %" PRIu64 ": Hash=", i);
                        //     for (int j = 0; j < 32; j++)
                        //     {
                        //         fprintf(stderr, "%02x", object_hash[j]);
                        //     }
                        //     fprintf(stderr, "\n");
                        // }
                        // getdata送信スレッドにinvベクタを転送する
                    }
                    else if (strcmp(command, "ping") == 0)
                    {
                        fprintf(stderr, "Received ping message\n");
                        // pingに対する処理をここに書く
                        // pongコマンドを送信する
                        unsigned char verack_header[24];
                        memcpy(verack_header, magicbytes, 4);
                        char verack_command[12] = {0};
                        strncpy(verack_command, "pong", 12);
                        memcpy(verack_header + 4, verack_command, 12);
                        uint32_t verack_payload_length = 0;
                        uint32_t net_verack_payload_length = htobe32(verack_payload_length);
                        memcpy(verack_header + 16, &net_verack_payload_length, 4);
                        unsigned char verack_checksum[4] = {0xcf, 0x83, 0xe1, 0x35}; // verackのペイロードは空なのでSHA512("")の最初の4バイト
                        memcpy(verack_header + 20, verack_checksum, 4);
                        ssize_t w = write(sock, verack_header, 24);
                        if (w != 24)
                        {
                            perror("write pong error");
                            close(sock);
                            break;
                        }
                    }
                    else if (strcmp(command, "getdata") == 0)
                    {
                        fprintf(stderr, "Received getdata message\n");
                    }
                    else if(strcmp(command, "object") == 0)
                    {
                        fprintf(stderr, "Received object message\n");
                        // object payload保存スレッドに転送する
                    }
                    else
                    {
                        fprintf(stderr, "Unknown command: %s\n", command);
                    }
                    // 処理が完了
                    // fprintf(stderr, "バッファに残ったペイロードデータの長さ: %zu, ", length - (24 + payload_length));
                    // fprintf(stderr, "Processed command: %s, payload length: %d\n", command, payload_length);
                    // 次のメッセージに備えてバッファを調整
                    memmove(connectedBuffer, connectedBuffer + 24 + payload_length, length - (24 + payload_length));
                    length -= (24 + payload_length);
                }
                // 再度読み込みが発生することがあるのでバッファをリセットせずにループに戻る
            }
        }
    }
    return EXIT_SUCCESS;
}
