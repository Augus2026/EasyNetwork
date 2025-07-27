#include "stage.h"

void print_hex(const char* data, int size) {
	printf("data len = %d hex data: ", size);
	for (int i = 0; i < size; i++) {
		printf("%02X ", (unsigned char)data[i]);
	}
	printf("\n");	
}

int read_peer_data(WOLFSSL* ssl, char* data, int size) {
	int received = 0;
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

void handle_register_peer(client_info_t* client, message_head_t* msg, int msglen) {
    memcpy(&client->register_addr, &msg->head.register_addr, sizeof(struct in_addr));
}

void handle_transport_data(client_info_t* client, message_head_t* msg, int msglen) {
	client_info_t* target_client = NULL;
	target_client = find_client_by_addr(&msg->head.dst_addr);			
	if (target_client) {
		int retval = wolfSSL_write(target_client->ssl, msg, sizeof(message_head_t) + msg->head.size);
		if(retval < 0) {
			printf("write message error. \r\n");
		}
		// printf("find client. \r\n");
	} else {
		// printf("no client found. \r\n");
	}
}

void handle_ping(client_info_t* client, message_head_t* msg, int msglen) {
    // update last recv time
    client->last_recv_time = time(0);
    // send pong message
	msg->head.msgtype = PONG;
	int retval = wolfSSL_write(client->ssl, msg, sizeof(message_head_t));
	if(retval < 0) {
		printf("write message error. \r\n");
	}
}
