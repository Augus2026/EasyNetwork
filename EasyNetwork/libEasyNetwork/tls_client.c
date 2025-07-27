#include "tls_client.h"
#include "hdr.h"

void print_hex(const char* data, int size) {
	printf("data len = %d hex data: ", size);
	for (int i = 0; i < size; i++) {
		printf("%02X ", (unsigned char)data[i]);
	}
	printf("\n");	
}

int read_peer_data(WOLFSSL* ssl, char* data, int size) {
	int received = -1;
	int i = 0;
	while (i < size) {
		int sz = size - i;
		received = wolfSSL_read(ssl, &data[i], sz);
		if (received > 0) {
			i += received;
		} else {
			int err = wolfSSL_get_error(ssl, received);
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

void MY_SSL_Init(peer_info_t* peer)
{
	// 初始化 Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed\n");
		return;
	}

	// 初始化 wolfSSL 库
	wolfSSL_Init();

	// 创建 SSL 上下文
	WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
	if (ctx == NULL) {
		printf("Error creating SSL context\n");
		WSACleanup();
		return;
	}

	// 加载 CA 证书以验证服务器证书
	if (wolfSSL_CTX_load_verify_locations(ctx, "ca-cert.pem", NULL) != SSL_SUCCESS) {
		printf("Error loading CA certificate\n");
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}

	// 创建 TCP 套接字
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET) {
		printf("Socket creation failed: %d\n", WSAGetLastError());
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}

	// 连接到服务器
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

	// 创建 SSL 对象
	WOLFSSL* ssl = wolfSSL_new(ctx);
	if (ssl == NULL) {
		printf("Error creating SSL object\n");
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		return;
	}
	// 将 SSL 对象与套接字关联
	wolfSSL_set_fd(ssl, (int)sockfd);

	// 发起 TLS/SSL 握手
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

void MY_SSL_Reconnect(peer_info_t* peer)
{
	MY_SSL_Cleanup(peer);
	MY_SSL_Init(peer);
	return;
}

// SSL 清理
void MY_SSL_Cleanup(peer_info_t* peer)
{
	if (peer->ssl) {
		printf("SSL connection already exists, reconnecting...\n");
		WOLFSSL_CTX* ctx = wolfSSL_get_SSL_CTX(peer->ssl);
		int sockfd = wolfSSL_get_fd(peer->ssl);
		wolfSSL_free(peer->ssl);
		closesocket(sockfd);
		wolfSSL_CTX_free(ctx);
		WSACleanup();
		peer->ssl = NULL;
	}
}
