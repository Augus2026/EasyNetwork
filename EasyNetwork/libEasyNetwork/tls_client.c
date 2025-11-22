#include "tls_client.h"
#include "hdr.h"

#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

void print_hex(const char* data, int size) {
	printf("data len = %d hex data: ", size);
	for (int i = 0; i < size; i++) {
		printf("%02X ", (unsigned char)data[i]);
	}
	printf("\n");	
}

void ssl_init(peer_info_t* peer) {
	// init Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed\n");
		return;
	}

	// init wolfSSL library
	wolfSSL_Init();

	// create wolfSSL context
	WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
	if (ctx == NULL) {
		printf("Error creating SSL context\n");
		WSACleanup();
		return;
	}

	char path[MAX_PATH] = "";
	char exePath[MAX_PATH] = "";
	GetModuleFileNameA(NULL, exePath, MAX_PATH);
	PathRemoveFileSpecA(exePath);
	snprintf(path, sizeof(path), "%s\\ca-cert.pem", exePath);
	printf("ca cert path: %s\n", path);

	// load CA certificate to verify server certificate
	if (wolfSSL_CTX_load_verify_locations(ctx, path, NULL) != SSL_SUCCESS) {
		printf("Error loading CA certificate\n");
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}

	// create tcp socket
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET) {
		printf("Socket creation failed: %d\n", WSAGetLastError());
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}

	// connect to server
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(peer->server_port);
	inet_pton(AF_INET, peer->server_ip, &server_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Connection failed: %d\n", WSAGetLastError());
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}
	printf("Connected to server\n");

	// create wolfSSL object
	WOLFSSL* ssl = wolfSSL_new(ctx);
	if (ssl == NULL) {
		printf("Error creating SSL object\n");
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}
	wolfSSL_set_fd(ssl, (int)sockfd);

	// connect wolfSSL
	if (wolfSSL_connect(ssl) != SSL_SUCCESS) {
		int err = wolfSSL_get_error(ssl, 0);
		printf("SSL connect failed, error: %d\n", err);
		wolfSSL_free(ssl);
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}
	peer->ssl = ssl;

	printf("SSL connection established\n");
	return;
}

void ssl_reconnect(peer_info_t* peer) {
	ssl_cleanup(peer);
	ssl_init(peer);
	return;
}

void ssl_cleanup(peer_info_t* peer) {
	if (peer->ssl) {
		printf("wolfSSL connection already exists, reconnecting...\n");
		WOLFSSL_CTX* ctx = wolfSSL_get_SSL_CTX(peer->ssl);
		int sockfd = wolfSSL_get_fd(peer->ssl);
		wolfSSL_free(peer->ssl);
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		peer->ssl = NULL;
	}
}

int ssl_read_data(peer_info_t* peer, char* data, int size) {
	int received = -1;
	int i = 0;
	while (i < size) {
		int sz = size - i;
		received = wolfSSL_read(peer->ssl, &data[i], sz);
		if (received > 0) {
			i += received;
		} else {
			int err = wolfSSL_get_error(peer->ssl, received);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			printf("Connection closed by server, error: %d\n", err);
			received = -1;
			break;
		}
	}
	return received;
}

int ssl_write_data(peer_info_t* peer, char* data, int size) {
	int sent = -1;
	int i = 0;
	while (i < size) {
		int sz = size - i;
		sent = wolfSSL_write(peer->ssl, &data[i], sz);
		if (sent > 0) {
			i += sent;
		} else {
			int err = wolfSSL_get_error(peer->ssl, sent);
			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			printf("Connection closed by server, error: %d\n", err);
			sent = -1;
			break;
		}
	}
	return sent;
}

message_head_t* recv_message(peer_info_t* peer) {
	int retval = 0;
    message_head_t* msg = (message_head_t*)malloc(sizeof(message_head_t));
        
	// read message head
	retval = ssl_read_data(peer, (char*)msg, sizeof(message_head_t));
	if(retval < 0) {
		printf("read package header error.\n");
		free(msg);
		return NULL;
	}
	// read message data
	if (msg->head.size > 0) {
		msg = (message_head_t*)realloc(msg, sizeof(message_head_t) + msg->head.size);
		retval = ssl_read_data(peer, msg->data, msg->head.size);
		if(retval < 0) {
			printf("read package data error.\n");
			free(msg);
			return NULL;
		}
	}
	return msg;
}

int send_message(peer_info_t* peer, const char* buffer, int len) {
	message_head_t* msg = (message_head_t*)malloc(sizeof(message_head_t) + len);
	memset(msg, 0, sizeof(message_head_t));
	msg->head.magic = 0x1234;
	msg->head.msgtype = TRANSPORT_DATA;
	msg->head.size = len;
	memcpy(msg->data, buffer, len);

	// read ip header
	ip_hdr* ip4_hdr = (ip_hdr*)buffer;
	if (len >= 20) {
		memcpy(&msg->head.src_addr, &ip4_hdr->srcaddr, sizeof(ip4_hdr->srcaddr));
		memcpy(&msg->head.dst_addr, &ip4_hdr->dstaddr, sizeof(ip4_hdr->dstaddr));
	} else {
		printf("Invalid IP packet header length\n");
		free(msg);
		return -1;
	}

	// write message
	int ret = ssl_write_data(peer, (char*)msg, sizeof(message_head_t) + len);
	if (ret < 0) {
		printf("Send data failed, error: %d\n", wolfSSL_get_error(peer->ssl, ret));
		free(msg);
		return -1;
	}

	free(msg);
	return 0;
}

void
PrintPacket(_In_ const BYTE *Packet, _In_ DWORD PacketSize)
{
    if (PacketSize < 20)
    {
        Log(WINTUN_LOG_INFO, L"Received packet without room for an IP header");
        return;
    }
    BYTE IpVersion = Packet[0] >> 4, Proto;
    WCHAR Src[46], Dst[46];
    if (IpVersion == 4)
    {
        RtlIpv4AddressToStringW((struct in_addr *)&Packet[12], Src);
        RtlIpv4AddressToStringW((struct in_addr *)&Packet[16], Dst);
        Proto = Packet[9];
        Packet += 20, PacketSize -= 20;
    }
    else if (IpVersion == 6 && PacketSize < 40)
    {
        Log(WINTUN_LOG_INFO, L"Received packet without room for an IP header");
        return;
    }
    else if (IpVersion == 6)
    {
        RtlIpv6AddressToStringW((struct in6_addr *)&Packet[8], Src);
        RtlIpv6AddressToStringW((struct in6_addr *)&Packet[24], Dst);
        Proto = Packet[6];
        Packet += 40, PacketSize -= 40;
    }
    else
    {
        Log(WINTUN_LOG_INFO, L"Received packet that was not IP");
        return;
    }
    if (Proto == 1 && PacketSize >= 8 && Packet[0] == 0)
        Log(WINTUN_LOG_INFO, L"Received IPv%d ICMP echo reply from %s to %s", IpVersion, Src, Dst);
    else
        Log(WINTUN_LOG_INFO, L"Received IPv%d proto 0x%x packet from %s to %s", IpVersion, Proto, Src, Dst);
}

void
RegisterPeer(peer_info_t* peer) {
    message_head_t* msg = (message_head_t*)malloc(sizeof(message_head_t));
    memset(msg, 0, sizeof(message_head_t));
    msg->head.magic = 0x1234;
    msg->head.msgtype = REGISTER_PEER;
    msg->head.register_addr.s_addr = inet_addr(peer->tunnel_ip);
    msg->head.size = 0;

    int ret = ssl_write_data(peer, (char*)msg, sizeof(message_head_t));
    if (ret <= 0) {
        printf("Send data failed, error: %d\n", wolfSSL_get_error(peer->ssl, ret));               
    }
    free(msg);
}