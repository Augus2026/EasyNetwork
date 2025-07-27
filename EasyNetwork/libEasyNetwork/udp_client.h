#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "message.h"
#include "kcp_client.h"

typedef void (msg_handler_t)(peer_info_t* peer, message_head_t* msg, int msglen);

int udp_send(peer_info_t* peer, const char* buf, int len);
int udp_recv(peer_info_t* peer, char* buf, int len);

void kcp_client_new(peer_info_t* peer);
void kcp_client_close(peer_info_t* peer);
void kcp_recv_once(peer_info_t* peer);

#endif // UDP_CLIENT_H
