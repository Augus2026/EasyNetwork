#ifndef CLIENT_MGR_H_
#define CLIENT_MGR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define WOLFSSL_USER_SETTINGS
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <uthash.h>
#include <wolfssl/ssl.h>
#include "message.h"

typedef struct client_info_ {
    char* key;

#ifdef _WIN32
	// 客户端文件描述符
	SOCKET clientfd;
#else
	// 客户端文件描述符
	int clientfd;
#endif

	// 客户端地址信息
	struct sockaddr_in remote_addr;
	int remote_addr_len;
	// peer注册信息
	struct in_addr register_addr;
	// SSL上下文
	WOLFSSL_CTX* ctx;
	WOLFSSL* ssl;

	// 最近一次接收数据时间
	time_t last_recv_time;

	UT_hash_handle hh;
} client_info_t;

void print_clients();
void add_client(client_info_t* client);
client_info_t* find_client(const char* key);
client_info_t* find_client_by_addr(const struct in_addr* addr);
void delete_client(client_info_t* client);

#endif // CLIENT_MGR_H_
