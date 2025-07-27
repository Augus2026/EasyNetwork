#include "udp_server.h"

#define KCP_KEY 0x11223344

char* gen_key(struct sockaddr_in* addr) {
    char ip[INET_ADDRSTRLEN] = "";
    unsigned short port = 0;
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
    port = ntohs(addr->sin_port);
    char key[1024] = "";
    sprintf(key, "%s:%d", ip, port);
    return strdup(key);
}

int udp_recv(server_info_t* server, char* buf, int len, struct sockaddr_in* addr, int* addr_len) {
    int retval = recvfrom(server->sockfd, buf, len, 0, (struct sockaddr*)addr, addr_len);
    if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAECONNRESET) {
			printf("connect reset \r\n");
			Sleep(1000);
		} else if (error == WSAEWOULDBLOCK) { // 设置非阻塞模式
            // printf("would block \r\n");
        } else if(error == WSAETIMEDOUT) { // 设置超时
			// printf("timeout \r\n");
		} else {
			printf("recvfrom() failed with error: %d\n", error);
		}
        return -1;
    }
    return retval;
}

int udp_send(client_info_t* client, const char* buf, int len) {
	int retval = sendto(client->sockfd, buf, len, 0, &(client->remote_addr), client->remote_addr_len);
	if (retval == SOCKET_ERROR) {
		printf("sendto() failed with error: %d\n", WSAGetLastError());
        return -1;
	}
    client->last_send_time = time(0);
	return retval;
}

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
	client_info_t* client = (client_info_t*)user;
    return udp_send(client, buf, len);
}

void handle_kcp_register_peer(client_info_t* client, kcp_message_t* msg, int msglen) {
    memcpy(&client->register_addr, &msg->head.register_addr, sizeof(struct in_addr));
}

void handle_kcp_transport_data(client_info_t* client, kcp_message_t* msg, int msglen) {
    client_info_t* target_client = NULL;
    target_client = find_client_by_addr(&msg->head.dst_addr);
    if (target_client) {
        // printf("[transport data] find client. \r\n");
        ikcp_send(target_client->kcp, msg, msglen);
    } else {
        // printf("[transport data] no client found. \r\n");
    }
}

void handle_kcp_ping(client_info_t* client, kcp_message_t* msg, int msglen) {
    // update last recv time
    client->last_recv_time = time(0);
    // send pong message
	msg->head.msgtype = KCP_PONG;
	ikcp_send(client->kcp, msg, msglen);
}

void kcp_server_new(server_info_t* server) {
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// create socket
	SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    server->sockfd = sockfd;

    // bind
	struct sockaddr_in local_addr;
	int local_addr_len = sizeof(local_addr);
	memset(&local_addr, 0, local_addr_len);
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(server->server_port);
	local_addr.sin_addr.s_addr = inet_addr(server->server_ip);
    bind(sockfd, (struct sockaddr*)&local_addr, local_addr_len);

    // 设置非阻塞模式
    u_long non_blocking = 1;
    if (ioctlsocket(sockfd, FIONBIO, &non_blocking) != 0) {
        printf("设置非阻塞模式失败，错误码: %d\n", WSAGetLastError());
    }

    printf("kcp server bind %s:%d \r\n", server->server_ip, server->server_port);
}

void kcp_server_close(server_info_t* server) {
	closesocket(server->sockfd);
	WSACleanup();
}

int udp_recv_once(server_info_t* server) {
    char data[1500];
    int size = 1500;

    // unsigned int clock1 = iclock();

    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    int len = udp_recv(server, data, size, &addr, &addr_len);
    if(len < 0) {
        return -1;
    }

#if 0
    // 清理不活跃的客户端
    clear_inactive_client();
#endif
    
    // 查找客户端
    char* key = gen_key(&addr);
    client_info_t* client = NULL;
    client = find_client(key);
    if(client == NULL) {
        client = (struct client_info_t*)malloc(sizeof(client_info_t));
        memset(client, 0, sizeof(client_info_t));

        client->sockfd = server->sockfd;
        memcpy(&client->remote_addr, &addr, addr_len);
        client->remote_addr_len = addr_len;
        client->last_recv_time = time(0);
		client->key = key;

        // create kcp
        ikcpcb *kcp = ikcp_create(KCP_KEY, (void*)client);
        ikcp_wndsize(kcp, 1024, 1024);
        ikcp_nodelay(kcp, 1, 80, 2, 1);
        // ikcp_interval(kcp, 10);
        ikcp_setoutput(kcp, udp_output);
        client->kcp = kcp;

        add_client(client);
    }

    int ret = ikcp_input(client->kcp, data, len);
    while (1) {
        int peeksize = ikcp_peeksize(client->kcp);
        if(peeksize < sizeof(message_head_t)) {
            break;
        }
        int msglen = peeksize;
        message_head_t* msg = (message_head_t*)malloc(msglen);

        int len = ikcp_recv(client->kcp, msg, msglen);
        if (len <= 0) {
            break;
        }
        assert(len == msglen);

        switch (msg->head.msgtype) {
        case REGISTER_PEER: {
            // printf("register peer. len = %d \n", msglen);
            handle_kcp_register_peer(client, msg, msglen);
            break;
        }
        case TRANSPORT_DATA: {
            // printf("transport data. len = %d data = %s \n", msglen, msg->data);
            handle_kcp_transport_data(client, msg, msglen);
            break;
        }
        case PING: {
            // printf("ping. len = %d code = %d \n", msglen, msg->head.code);
            handle_kcp_ping(client, msg, msglen);
            break;
        }
        default:
            printf("unregister msg type = %d \n", msg->head.msgtype);
            break;
        }
    }
    ikcp_flush(client->kcp);
    
    // unsigned int clock2 = iclock();
    // printf("diff4: %dms \r\n", clock2 - clock1);
    return 0;
}
