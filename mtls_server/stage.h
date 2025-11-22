#ifndef __STAGE_H__
#define __STAGE_H__

#include <wolfssl/ssl.h>

#include "client_mgr.h"
#include "message.h"

void print_hex(const char* data, int size);
int read_peer_data(WOLFSSL* ssl, char* data, int size);

void handle_register_peer(client_info_t* client, message_head_t* msg, int msglen);
void handle_transport_data(client_info_t* client, message_head_t* msg, int msglen);
void handle_ping(client_info_t* client, message_head_t* msg, int msglen);

#endif // __STAGE_H__