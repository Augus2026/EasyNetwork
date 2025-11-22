#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"
#include "client_mgr.h"
#include "stage.h"
#include "log.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#endif

#define DEFAULT_SERVER_IP "0.0.0.0"
#define DEFAULT_SERVER_PORT 1001

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void* handle_client(void* arg) {
#endif
    client_info_t* client = (client_info_t*)arg;
#ifdef _WIN32
	SOCKET clientfd = client->clientfd;
#else
	int clientfd = client->clientfd;
#endif
	WOLFSSL_CTX* ctx = client->ctx;

    // 创建 SSL 对象
	WOLFSSL* ssl = wolfSSL_new(ctx);
	if (ssl == NULL) {
		log_error("Error creating SSL object");
		goto done;
	}
	// 将 SSL 对象与客户端套接字关联
	wolfSSL_set_fd(ssl, (int)clientfd);
	client->ssl = ssl;
	add_client(client);

	int len = 0;
	while(1) {
		// 读取数据头
		int msglen = sizeof(message_head_t);
		message_head_t* msg = (message_head_t*)malloc(msglen);
		len = read_peer_data(ssl, (char*)msg, msglen);
		if(len < 0) {
			log_error("read package header error");
			free(msg);
			break;
		}
		msglen += msg->head.size;

		if(msg->head.magic != 0x1234) {
			log_warn("magic error. magic = 0x%x", msg->head.magic);
			free(msg);
			break;
		}

		switch (msg->head.msgtype) {
		case REGISTER_PEER: {
			log_debug("register peer");
			handle_register_peer(client, msg, msglen);
			break;
		}
		case TRANSPORT_DATA: {
			log_debug("transport data");
			msg = (message_head_t*)realloc(msg, msglen);
			len = read_peer_data(ssl, (char*)msg->data, msg->head.size);
			if(len < 0) {
				log_error("read package data error");
				break;
			}
			handle_transport_data(client, msg, msglen);
			break;
		}
		case PING: {
			log_debug("ping");
			handle_ping(client, msg, msglen);
			break;
		}
		default:
			log_warn("msgtype error. msgtype = %d", msg->head.msgtype);
			break;
		}
		free(msg);
	}

done:
	if (ssl) {
        wolfSSL_shutdown(ssl);
        wolfSSL_free(ssl);
    }

	if (clientfd != -1) {
        CloseSocket(clientfd);
    }

	delete_client(client);
	log_info("Client disconnected");
	return 0;
}

int main(int argc, char* argv[]) {
	FILE* fp = fopen("mtls_server.log", "w");
    if (!fp) {
        log_error("Failed to open mtls_server.log");
        return -1;
    }
    log_set_level(LOG_INFO);
	log_add_fp(fp, LOG_INFO);

    log_info("Starting mTLS server...");
    
	char* server_ip = DEFAULT_SERVER_IP;
	int server_port = DEFAULT_SERVER_PORT;
    char* ca_cert_path = "/etc/easynet/certs/server-cert.pem";
    char* ca_key_path = "/etc/easynet/certs/server-key.pem";
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ca-path") == 0 && i + 1 < argc) {
            ca_cert_path = argv[++i];
        } else if (strcmp(argv[i], "--ca-key") == 0 && i + 1 < argc) {
            ca_key_path = argv[++i];
        } else if (strcmp(argv[i], "--server-ip") == 0 && i + 1 < argc) {
            server_ip = argv[++i];
        } else if (strcmp(argv[i], "--server-port") == 0 && i + 1 < argc) {
            server_port = atoi(argv[++i]);
        } else {
			log_error("Unknown argument: %s", argv[i]);
			return -1;
		}
    }

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		log_error("WSAStartup failed");
		return -1;
	}
	SOCKET sockfd = INVALID_SOCKET;
#else
	int sockfd = 0;
#endif

	// 初始化 wolfSSL 库
	wolfSSL_Init();

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(server_port);

	// 初始化 wolfSSL 库
	wolfSSL_Init();

	// 创建 SSL 上下文
	WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_2_server_method());
	if (ctx == NULL) {
		log_error("Error creating SSL context");
		goto done;
	}

	// 加载服务器证书
	if (wolfSSL_CTX_use_certificate_file(ctx, ca_cert_path, SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		log_error("Error loading server certificate from %s", ca_cert_path);
		goto done;
	}

	// 加载服务器私钥
	if (wolfSSL_CTX_use_PrivateKey_file(ctx, ca_key_path, SSL_FILETYPE_PEM) != SSL_SUCCESS) {
		log_error("Error loading server private key from %s", ca_key_path);
		goto done;
	}

	// 创建 TCP 套接字
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0) {
		log_error("Socket creation failed: %d", errno);
		goto done;
	}

	// 绑定套接字
	if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		log_error("Bind failed: %d", errno);
		goto done;
	}

	// 监听连接
	if (listen(sockfd, 1) < 0) {
		log_error("Listen failed: %d", errno);
		goto done;
	}
	log_info("Server listening on %s:%d", server_ip, server_port);

	while (1) {
		// 接受客户端连接
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
		if (clientfd < 0) {
			log_error("Accept failed: %d", errno);
			goto done;
		}
		log_info("Client connected");

        // 创建客户端信息结构体
        client_info_t* client = malloc(sizeof(client_info_t));
		memset(client, 0, sizeof(client_info_t));
		char ip[INET_ADDRSTRLEN] = "";
		unsigned short port = 0;
		inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
		port = ntohs(client_addr.sin_port);
		char key[1024] = "";
		sprintf(key, "%s:%d", ip, port);
		client->key = strdup(key);
        client->clientfd = clientfd;
		memcpy(&client->remote_addr, &client_addr, sizeof(struct sockaddr_in));
		client->remote_addr_len = client_len;
		client->ctx = ctx;

#ifdef _WIN32
        // 为每个客户端创建新线程
        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, (LPVOID)client, 0, NULL);
        if (thread == NULL) {
            log_error("CreateThread failed: %d", GetLastError());
            CloseSocket(clientfd);
            continue;
        }
        CloseHandle(thread);
#else
		// 为每个客户端创建新线程
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void*)client) != 0) {
            log_error("pthread_create failed");
            CloseSocket(clientfd);
            continue;
        }
#endif
	}
	
done:
    if (sockfd != -1) {
        CloseSocket(sockfd);
    }
    if (ctx) {
        wolfSSL_CTX_free(ctx);
    }
    wolfSSL_Cleanup();
#ifdef _WIN32
	WSACleanup();
#endif

    log_info("Server shutdown");
	fclose(fp);
    return 0;
}