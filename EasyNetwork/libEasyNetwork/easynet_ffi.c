#include "easynet_ffi.h"
#include <stdio.h>
#include <string.h>

#include "wintun_peer.h"
#include "itf.h"

#include "logc/log.h"

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

FILE *fp = NULL;

void log_init() {
    fp = fopen("log.txt", "a+");
    if(fp == NULL){
        printf("create log file failed.\n");
        return -1;
    }
    log_set_level(LOG_TRACE);
    log_add_fp(fp, LOG_INFO);
}

void log_clean() {
    fclose(fp);
    fp = NULL;
}

void init_easynet() {
    log_init();
}

void clean_easynet() {
    log_clean();
}

void set_reply_server(
    const char* reply_address,
    int reply_port) {
    init_easynet();

    log_info("Setting server:");
    log_info("Reply Address: %s", reply_address);
    log_info("Reply Port: %d", reply_port);

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
    log_info("Joining network:");
    log_info("Interface: %s", ifname);
    log_info("Description: %s", ifdesc);
    log_info("IP: %s", ip);
    log_info("Netmask: %s", netmask);
    log_info("MTU: %s", mtu);
    log_info("Domain: %s", domain);
    log_info("Name Server: %s", nameServer);
    log_info("Search List: %s", searchList);
    
    peer = (peer_info_t*)malloc(sizeof(peer_info_t));
    memset(peer, 0, sizeof(peer_info_t));

    peer->server_ip = _strdup(server_info.reply_address);
    peer->server_port = server_info.reply_port;
    peer->name = _strdup(ifname);
    peer->desc = _strdup(ifdesc);
    peer->tunnel_ip = _strdup(ip);
    peer->tunnel_mask = atoi(netmask);
    peer->mtu = atoi(mtu);
    peer->domain = _strdup(domain);
    peer->name_server = _strdup(nameServer);
    peer->search_list = _strdup(searchList);

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
    log_info("Leaving network on interface: %s", ifname);
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
    log_info("add_route() destination: %s netmask: %s gateway: %s metric: %s",
        destination, netmask, gateway, metric);

     DWORD dwForwardMetric1 = atoi(metric);
     AddRoute(L"EasyNetwork", destination, netmask, gateway, dwForwardMetric1);
}

void clean_route()
{
    log_info("clean_route()");
    CleanRoute(L"EasyNetwork");
}