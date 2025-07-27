#ifndef TLS_CLIENT_H_
#define TLS_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define WOLFSSL_USER_SETTINGS
#include "wolfssl/ssl.h"
#include "ikcp.h"
#include "wintun_peer.h"

typedef struct peer_info_ {
    char* server_ip; // 服务器IP地址
    int server_port; // 服务器端口

    wchar_t* name; // 网卡名称
    wchar_t* desc; // 网卡描述
    char* tunnel_ip; // 网卡IP地址
    int tunnel_mask; // 网卡掩码

    int mtu; // 最大传输单元
    char* domain; // 域名
    char* name_server; // 域名服务器
    char* search_list; // 搜索列表

	// wintun
	WINTUN_SESSION_HANDLE Session; // wintun会话句柄

	// tls
    WOLFSSL* ssl; // TLS连接

	// kcp
	SOCKET sockfd;
	ikcpcb *kcp;
    struct sockaddr_in remote_addr; // 服务器地址
    int remote_addr_len; // 服务器地址长度

    // 发送队列
    CRITICAL_SECTION snd_queue_cs;
    iqueue_head snd_queue;
    // 接收队列
    CRITICAL_SECTION rcv_queue_cs;
    iqueue_head rcv_queue;

	unsigned int code;
} peer_info_t;

int read_peer_data(WOLFSSL* ssl, char* data, int size);

// SSL 连接
void MY_SSL_Init(peer_info_t* peer);
// SSL 重连
void MY_SSL_Reconnect(peer_info_t* peer);
// SSL 清理
void MY_SSL_Cleanup(peer_info_t* peer);

#endif // TLS_CLIENT_H_
