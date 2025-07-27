#include "client_mgr.h"
#include "udp_server.h"

#define SERVER_IP "0.0.0.0"
#define SERVER_PORT 1234

int main(int argc, char* argv[]) {
	server_info_t server;
	server.server_ip = SERVER_IP;
	server.server_port = SERVER_PORT;
	kcp_server_new(&server);

	fd_set readfds;
	struct timeval timeout;

	while (1) {
        unsigned int clock1 = iclock();

        // 清理不活跃的客户端
        // clear_inactive_client();

        update_all_kcp();

        timeBeginPeriod(1);
        // 初始化文件描述符集合
        FD_ZERO(&readfds);
        FD_SET(server.sockfd, &readfds);
        // 设置超时时间
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        int activity = select(server.sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == -1) {
            // 处理错误
            perror("select error");
        } else if (activity == 0) {
            unsigned int clock2 = iclock();
            // printf("server once: %dms \r\n", clock2 - clock1);
            // 超时，没有数据可读，继续下一次循环
            continue;
        } else {
            // 有数据可读，处理 UDP 数据
            udp_recv_once(&server);
        }
        timeEndPeriod(1);
	}
	kcp_server_close(&server);
	return 0;
}