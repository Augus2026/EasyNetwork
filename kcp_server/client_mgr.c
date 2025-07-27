#include "client_mgr.h"

client_info_t* clients = NULL;

void print_clients() {
    client_info_t* client = NULL;
    client_info_t* tmp = NULL;
    printf("=========================================================\r\n");
    HASH_ITER(hh, clients, client, tmp) {
        printf("client key: %s\n", client->key);
        printf("client fd: %d\n", (int)client->sockfd);

        char register_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client->register_addr, register_addr, INET_ADDRSTRLEN);
        printf("client tunnel src addr: %s\n", register_addr);
    }
    printf("=========================================================\r\n");
}

void add_client(client_info_t* client) {
    HASH_ADD_STR(clients, key, client);
    // print_clients();
}

client_info_t *find_client(char* key) {
	client_info_t *client = NULL;
	HASH_FIND_STR(clients, key, client);
    // print_clients();
	return client;
}

client_info_t* find_client_by_addr(struct in_addr* addr) {
    client_info_t* client = NULL;
    client_info_t* tmp = NULL;
    HASH_ITER(hh, clients, client, tmp) {
        if (client->register_addr.s_addr == addr->s_addr) {
            break;
        }
    }
	// print_clients();
    return client;
}

void delete_client(client_info_t* client) {
    HASH_DEL(clients, client);
    free(client->key);
    free(client);
    // print_clients();
}

void clear_inactive_client() {
	client_info_t *client = clients;
	while (client != NULL) {
		int cur = time(0);
		int timeout = 5;

		// recv timeout
		client_info_t* tmp = NULL;
		if (client->last_recv_time != 0 && cur - client->last_recv_time > timeout) {
			ikcp_release(client->kcp);
			HASH_DEL(clients, client);

			tmp = (client_info_t*)(client->hh.next);
			free(client);
			client = tmp;

			printf("recv timeout \r\n");
			continue;
		}
		client = (client_info_t*)(client->hh.next);
	}
}

void update_all_kcp() {
    client_info_t* client = NULL;
    client_info_t* tmp = NULL;

    HASH_ITER(hh, clients, client, tmp) {
        IUINT32 current_clock = iclock();
        IUINT32 next_update = ikcp_check(client->kcp, current_clock);
        if (current_clock >= next_update) {
            ikcp_update(client->kcp, current_clock);
        }
    }
}
