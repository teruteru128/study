
#ifndef EPSP_DATA_TYPE_H
#define EPSP_DATA_TYPE_H

#include <stdint.h>

typedef struct server_point{
    char* address;
    uint16_t port;
} server_point;

typedef struct peer_data{
    server_point address;
    uint16_t peer_id;
} peer_data;

#endif
