#include "udp_client.h"

#include <time.h>
#include <stdio.h>
#include <windows.h>

#include "ikcp.h"
#define KCP_KEY 0x11223344

int udp_send(const peer_info_t* peer, const char* buf, int len) {
	int retval = sendto(peer->sockfd, buf, len, 0, (struct sockaddr*)&(peer->remote_addr), peer->remote_addr_len);
	if (retval == SOCKET_ERROR) {
		printf("sendto() failed with error: %d\n", WSAGetLastError());
        return -1;
	}
	return retval;
}

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
	peer_info_t* peer = (peer_info_t*)user;
    return udp_send(peer, buf, len);
}

void writelog(const char *log, struct IKCPCB *kcp, void *user) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time_str[80];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    DWORD threadId = GetCurrentThreadId();
    printf("[%s] [Thread ID: %lu] %s \r\n", time_str, threadId, log);
}

void reset_kcp_session(peer_info_t* peer) {
    ikcp_release(peer->kcp);
    peer->kcp = NULL;

    ikcpcb *kcp = ikcp_create(KCP_KEY, (void*)peer);
    ikcp_wndsize(kcp, 1024, 1024);
    ikcp_nodelay(kcp, 1, 1, 2, 1);
    ikcp_setoutput(kcp, udp_output);
    peer->kcp = kcp;
}

int udp_recv(peer_info_t* peer, char* buf, int len) {
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    int retval = recvfrom(peer->sockfd, buf, len, 0, (struct sockaddr*)&addr, &addr_len);
    if (retval == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAECONNRESET) {
            reset_kcp_session(peer);
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

void kcp_client_new(peer_info_t* peer) {
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// create socket
	SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    peer->sockfd = sockfd;

#if 1
    // 设置非阻塞模式
    u_long non_blocking = 1;
    if (ioctlsocket(sockfd, FIONBIO, &non_blocking) != 0) {
        printf("设置非阻塞模式失败，错误码: %d\n", WSAGetLastError());
    }
#endif

    // create kcp
    ikcpcb *kcp = ikcp_create(KCP_KEY, (void*)peer);
    ikcp_wndsize(kcp, 1024, 1024);
    ikcp_nodelay(kcp, 1, 80, 2, 1);
    // ikcp_interval(kcp, 10);
    kcp->output = udp_output;
    peer->kcp = kcp;

    // kcp->logmask = ~0;
    // kcp->writelog = writelog;

	// server address
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	memset(&addr, 0, addr_len);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(peer->server_ip);
    addr.sin_port = htons(peer->server_port);
    peer->remote_addr = addr;
    peer->remote_addr_len = addr_len;
}

void kcp_client_close(peer_info_t* peer) {
    ikcp_release(peer->kcp);
	closesocket(peer->sockfd);
	WSACleanup();
}

void kcp_recv_once(peer_info_t* peer) {
    char data[1500];
    int size = 1500;
    int len = udp_recv(peer, data, size);
    if(len < 0) {
        return;
    }

    ikcp_input(peer->kcp, data, len);
    while (1) {
        int peeksize = ikcp_peeksize(peer->kcp);
        if(peeksize < sizeof(message_head_t)) {
            break;
        }
        int msglen = peeksize;
        message_head_t* msg = (message_head_t*)malloc(msglen);
        int len = ikcp_recv(peer->kcp, (char*)msg, msglen);
        if (len <= 0) {
            break;
        }
        assert(len == msglen);

        switch(msg->head.msgtype) {
            case PONG: {
                unsigned int diff = iclock() - msg->head.clock;
                printf("pong. code %d diff: %dms. \r\n", msg->head.code, diff);
                break;
            }
            case TRANSPORT_DATA: {
                MsgNode* node = (MsgNode*)malloc(sizeof(MsgNode));
                node->msg = msg;
                node->msglen = msglen;
                node->timestamp = iclock();
                node->index = 0;
                EnterCriticalSection(&peer->rcv_queue_cs);
                iqueue_add_tail(&node->head, &peer->rcv_queue);
                LeaveCriticalSection(&peer->rcv_queue_cs);
                break;
            }
            default: {
                printf("unknown msgtype: %d \r\n", msg->head.msgtype);
                break;
            }
        }
    }
    ikcp_flush(peer->kcp);
}