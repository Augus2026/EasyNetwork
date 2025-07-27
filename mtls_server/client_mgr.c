#include "client_mgr.h"

client_info_t* clients = NULL;

void print_clients() {
    FILE *fp = fopen("clients.txt", "w");
    if (!fp) {
        printf("Failed to open clients.txt\n");
        return;
    }

    client_info_t* client = NULL;
    client_info_t* tmp = NULL;
    fprintf(fp, "=========================================================\r\n");
    HASH_ITER(hh, clients, client, tmp) {
        fprintf(fp, "client key: %s\n", client->key);
        fprintf(fp, "client fd: %d\n", (int)client->clientfd);
        fprintf(fp, "client ctx: %p\n", client->ctx);
        fprintf(fp, "client ssl: %p\n", client->ssl);

        // 打印远程地址和本地地址
        char remote_ip[INET_ADDRSTRLEN];
        unsigned short remote_port = 0;
        inet_ntop(AF_INET, &client->remote_addr.sin_addr, remote_ip, INET_ADDRSTRLEN);
        remote_port = ntohs(client->remote_addr.sin_port);
        fprintf(fp, "client remote addr: %s:%d\n", remote_ip, remote_port);

        // 打印隧道地址
        char register_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client->register_addr, register_addr, INET_ADDRSTRLEN);
        fprintf(fp, "client tunnel src addr: %s\n", register_addr);
    }
    fprintf(fp, "=========================================================\r\n");
    
    fclose(fp);
}

void add_client(client_info_t* client)
{
    HASH_ADD_STR(clients, key, client);
}

client_info_t* find_client(const char* key)
{
    client_info_t* client = NULL;
    HASH_FIND_STR(clients, key, client);
    return client;
}

client_info_t* find_client_by_addr(const struct in_addr* addr)
{
    client_info_t* client = NULL;
    client_info_t* tmp = NULL;
    HASH_ITER(hh, clients, client, tmp) {
        if (client->register_addr.s_addr == addr->s_addr) {
            break;
        }
    }
    return client;
}

void delete_client(client_info_t* client)
{
    HASH_DEL(clients, client);
    free(client->key);
    free(client);
}
