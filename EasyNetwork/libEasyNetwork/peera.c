#include "easy_network_ffi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    const char* server_ip = "127.0.0.1";
    int server_port = 1234;

    if (argc >= 3) {
        server_ip = argv[1];
        server_port = atoi(argv[2]);
        if (server_port <= 0 || server_port > 65535) {
            fprintf(stderr, "错误：无效的端口号，端口号范围应为 1 - 65535。\n");
            return 1;
        }
    } else if (argc > 1) {
        fprintf(stderr, "错误：需要同时提供服务器 IP 和端口号。\n");
        return 1;
    }

    set_server(server_ip, server_port);
    join_network("eth0", "EasyNetwork", "10.10.10.11", "255.255.255.0", "1400", "example.com", "8.8.8.8", "8.8.4.4");
    Sleep(3000);
    add_route("10.10.10.0", "255.255.255.0", "10.10.10.11", "100");

    getchar();
    return 0;
}