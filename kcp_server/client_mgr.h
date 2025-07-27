#ifndef _CLIENT_MGR_H_
#define _CLIENT_MGR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#include "ikcp.h"
#include "uthash.h"
#include "udp_server.h"

#define BUFFSIZE 4096
#define KCP_KEY 0x11223344

// client info
typedef struct client_info_ {
    char* key; // format: 192.168.10.1:1234

    SOCKET sockfd;
    ikcpcb *kcp;

    struct sockaddr_in remote_addr; // Remote address
    int remote_addr_len; // Remote address length

    struct in_addr register_addr; // Registration address info

    int last_send_time;
    int last_recv_time;

    UT_hash_handle hh; /* makes this structure hashable */
} client_info_t;

// KCP client info list
client_info_t* clients;

void print_clients();
void add_client(client_info_t* client);
client_info_t *find_client(char* key);
client_info_t* find_client_by_addr(struct in_addr* addr);
void delete_client(client_info_t* client);
void clear_inactive_client();
void update_all_kcp();

#endif // _CLIENT_MGR_H_
