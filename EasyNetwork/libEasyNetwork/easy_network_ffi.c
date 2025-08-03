#include "easy_network_ffi.h"
#include <stdio.h>
#include <string.h>

#include "wintun_peer.h"
#include "itf.h"

typedef struct server_info_ {
    char* reply_address;
    int reply_port;
} server_info_t;

peer_info_t* peer = NULL;
server_info_t server_info;

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    peer_info_t* peer = (peer_info_t*)lpParam;
    build_peer(peer);
    return 0;
}

void set_reply_server(
    const char* reply_address,
    int reply_port) {
    printf("Setting server:\n");
    printf("Reply Address: %s\n", reply_address);
    printf("Reply Port: %d\n", reply_port);

    server_info.reply_address = _strdup(reply_address);
    server_info.reply_port = reply_port;
}

void join_network(
    const char* ifname,
    const char* ifdesc,
    const char* ip,
    const char* netmask,
    const char* mtu,
    const char* domain,
    const char* nameServer,
    const char* searchList) {
    printf("Joining network:\n");
    printf("Interface: %s\n", ifname);
    printf("Description: %s\n", ifdesc);
    printf("IP: %s\n", ip);
    printf("Netmask: %s\n", netmask);
    printf("MTU: %s\n", mtu);
    printf("Domain: %s\n", domain);
    printf("Name Server: %s\n", nameServer);
    printf("Search List: %s\n", searchList);
    
    peer = (peer_info_t*)malloc(sizeof(peer_info_t));
    memset(peer, 0, sizeof(peer_info_t));

    peer->server_ip = _strdup(server_info.reply_address);
    peer->server_port = server_info.reply_port;
    peer->name = _wcsdup(L"EasyNetwork");
    peer->desc = _wcsdup(L"EasyNetwork Peer");
    peer->tunnel_ip = strdup(ip);
    peer->tunnel_mask = atoi(netmask);
    peer->mtu = atoi(mtu);
    peer->domain = strdup(domain);
    peer->name_server = strdup(nameServer);
    peer->search_list = strdup(searchList);

    // 创建线程执行build_peer
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, peer, 0, NULL);
    if (thread == NULL) {
        free(peer->server_ip);
        free(peer->name);
        free(peer->desc);
        free(peer->desc);
        free(peer->tunnel_ip);
        free(peer->domain);
        free(peer->name_server);
        free(peer->search_list);
        free(peer);
        return;
    }
    CloseHandle(thread);
    return;
}

void leave_network(
    const char* ifname) {
    printf("Leaving network on interface\n");
    destroy_peer();
}

void reset_network(
    const char* ifname
    ) {
    leave_network(ifname);

    // 创建线程执行build_peer
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, peer, 0, NULL);
    if (thread == NULL) {
        free(peer->server_ip);
        free(peer->name);
        free(peer->desc);
        free(peer->desc);
        free(peer->tunnel_ip);
        free(peer->domain);
        free(peer->name_server);
        free(peer->search_list);
        free(peer);
        return;
    }
    CloseHandle(thread);
}

void add_route(
    const char* destination,
    const char* netmask,
    const char* gateway,
    const char* metric)
{
    printf("add_route() destination: %s netmask: %s gateway: %s metric: %s \r\n",
        destination, netmask, gateway, metric);

     DWORD dwForwardMetric1 = atoi(metric);
     AddRoute(L"EasyNetwork", destination, netmask, gateway, dwForwardMetric1);
}

void clean_route()
{
    printf("clean_route() \r\n");
    CleanRoute(L"EasyNetwork");
}