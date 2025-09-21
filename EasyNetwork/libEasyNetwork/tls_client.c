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
