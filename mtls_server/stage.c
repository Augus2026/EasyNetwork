#include "stage.h"
#include "log.h"

void print_hex(const char* data, int size) {
    log_debug("data len = %d hex data: ", size);
    for (int i = 0; i < size; i++) {
        log_debug("%02X ", (unsigned char)data[i]);
    }
    log_debug("\n");	
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
			log_error("Connection closed by server, error: %d", err);
            received = -1;
			break;
		}
	}
	return received;
}

void handle_register_peer(client_info_t* client, message_head_t* msg, int msglen) {
    memcpy(&client->register_addr, &msg->head.register_addr, sizeof(struct in_addr));
    log_debug("Client registered with address: %s", inet_ntoa(client->register_addr));
}

void handle_transport_data(client_info_t* client, message_head_t* msg, int msglen) {
	client_info_t* target_client = NULL;
	target_client = find_client_by_addr(&msg->head.dst_addr);			
	if (target_client) {
		int retval = wolfSSL_write(target_client->ssl, msg, sizeof(message_head_t) + msg->head.size);
		if(retval < 0) {
			log_error("write message error");
		} else {
            log_debug("Data forwarded to target client, size: %d", msg->head.size);
        }
	} else {
		log_warn("No client found for address: %s", inet_ntoa(msg->head.dst_addr));
	}
}

void handle_ping(client_info_t* client, message_head_t* msg, int msglen) {
    // update last recv time
    client->last_recv_time = time(0);
    // send pong message
	msg->head.msgtype = PONG;
	int retval = wolfSSL_write(client->ssl, msg, sizeof(message_head_t));
	if(retval < 0) {
		log_error("write pong message error");
	} else {
        log_debug("Pong sent to client");
    }
}