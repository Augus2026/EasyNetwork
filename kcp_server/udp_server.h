#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "client_mgr.h"
#include "kcp_utils.h"

// server info
typedef struct server_info_ {
	SOCKET sockfd;

	char* server_ip; // server ip address
    int server_port; // server port

} server_info_t;

void kcp_server_new(server_info_t* server);
void kcp_server_close(server_info_t* server);

int udp_send(struct client_info_t* client, char* buf, int len);
int udp_recv_once(struct server_info_t* server);
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);

#endif // UDP_SERVER_H
