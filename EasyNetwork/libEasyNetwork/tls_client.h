#ifndef TLS_CLIENT_H_
#define TLS_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define WOLFSSL_USER_SETTINGS
#include "wolfssl/ssl.h"

#include "wintun_peer.h"
#include "message.h"

typedef struct peer_info_ {
    char* server_ip; // 服务器IP地址
    int server_port; // 服务器端口

    char* name; // 网卡名称
    char* desc; // 网卡描述
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

message_head_t* recv_message(peer_info_t* peer);
int send_message(peer_info_t* peer, const char* data, int size);

void
PrintPacket(_In_ const BYTE *Packet, _In_ DWORD PacketSize);

void
RegisterPeer(peer_info_t* peer);

#endif // TLS_CLIENT_H_
