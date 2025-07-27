#include "kcp_client.h"
#include "udp_client.h"

message_head_t* new_kcp_ping(peer_info_t* peer) {
    int msglen = sizeof(message_head_t);
    message_head_t* msg = (message_head_t*)malloc(msglen);
    memset(msg, 0, msglen);
	msg->head.magic = 0x1234;
	msg->head.msgtype = PING;
    msg->head.code = peer->code;
    msg->head.clock = iclock();
    msg->head.size = 0;
    return msg;
}

message_head_t* new_kcp_register_peer(peer_info_t* peer) {
    int msglen = sizeof(message_head_t);
    message_head_t* msg = (message_head_t*)malloc(msglen);
    memset(msg, 0, msglen);
	msg->head.magic = 0x1234;
	msg->head.msgtype = REGISTER_PEER;
	msg->head.register_addr.s_addr = inet_addr(peer->tunnel_ip);
    msg->head.size = 0;
    return msg;
}

message_head_t* new_kcp_transport_data(peer_info_t* peer, const char* data, int len) {
    ip_hdr* ip4_hdr = (ip_hdr*)data;
    if (len >= 20) {
        // todo
    } else {
        printf("Invalid IP packet header length\n");
        return -1;
    }

    int msglen = sizeof(message_head_t) + len;
    message_head_t* msg = (message_head_t*)malloc(msglen);
    memset(msg, 0, msglen);
    msg->head.magic = 0x1234;
	msg->head.msgtype = TRANSPORT_DATA;
	msg->head.size = len;
    msg->head.src_addr.s_addr = ip4_hdr->srcaddr.s_addr;
    msg->head.dst_addr.s_addr = ip4_hdr->dstaddr.s_addr;
	memcpy(msg->data, data, len);
    return msg;
}

void kcp_register_peer(peer_info_t* peer) {
    int msglen = sizeof(message_head_t);
    message_head_t* msg = (message_head_t*)malloc(msglen);
    memset(msg, 0, msglen);
	msg->head.magic = 0x1234;
	msg->head.msgtype = REGISTER_PEER;
	msg->head.register_addr.s_addr = inet_addr(peer->tunnel_ip);
    msg->head.size = 0;
	ikcp_send(peer->kcp, msg, msglen);
    free(msg);
}

void kcp_transport_data(peer_info_t* peer, const char* data, int len) {
    ip_hdr* ip4_hdr = (ip_hdr*)data;
    if (len >= 20) {
        // todo
    } else {
        printf("Invalid IP packet header length\n");
        return -1;
    }

    int msglen = sizeof(message_head_t) + len;
    message_head_t* msg = (message_head_t*)malloc(msglen);
    memset(msg, 0, msglen);
    msg->head.magic = 0x1234;
	msg->head.msgtype = TRANSPORT_DATA;
	msg->head.size = len;
    msg->head.src_addr.s_addr = ip4_hdr->srcaddr.s_addr;
    msg->head.dst_addr.s_addr = ip4_hdr->dstaddr.s_addr;
	memcpy(msg->data, data, len);
    ikcp_send(peer->kcp, msg, msglen);
    free(msg);
}
