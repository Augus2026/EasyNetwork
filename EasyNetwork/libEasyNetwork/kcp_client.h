#ifndef KCP_CLIENT_H
#define KCP_CLIENT_H

#include "message.h"
#include "kcp_utils.h"
#include "tls_client.h"

typedef struct MsgNode {
    message_head_t* msg;
    int msglen;
    IUINT32 timestamp;
    int index;
    iqueue_head head;
} MsgNode;

message_head_t* new_kcp_ping(peer_info_t* peer);
message_head_t* new_kcp_register_peer(peer_info_t* peer);
message_head_t* new_kcp_transport_data(peer_info_t* peer, const char* data, int len);

void kcp_register_peer(const peer_info_t* peer);
void kcp_transport_data(const peer_info_t* peer, const char* data, int len);

#endif // KCP_CLIENT_H