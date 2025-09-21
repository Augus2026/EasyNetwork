#ifndef TLS_CLIENT_H_
#define TLS_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "wintun_peer.h"

#define WOLFSSL_USER_SETTINGS
#include "wolfssl/ssl.h"

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

	WINTUN_SESSION_HANDLE Session; // wintun会话句柄
    WOLFSSL* ssl; // TLS连接
} peer_info_t;

// ssl init
void ssl_init(peer_info_t* peer);
// ssl reconnect
void ssl_reconnect(peer_info_t* peer);
// ssl cleanup
void ssl_cleanup(peer_info_t* peer);
// read data
int ssl_read_data(peer_info_t* peer, char* data, int size);
// write data
int ssl_write_data(peer_info_t* peer, char* data, int size);

#endif // TLS_CLIENT_H_
