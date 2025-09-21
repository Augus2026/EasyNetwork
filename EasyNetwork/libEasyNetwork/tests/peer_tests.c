#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "easynet_ffi.h"
#include "cJSON/cJSON.h"

typedef struct {
    struct {
        char* server_ip;
        int server_port;
    } srv;
    struct {
        char* ifname;
        char* ifdesc;
        char* ip;
        char* netmask;
        char* mtu;
        char* domain;
        char* nameServer;
        char* searchList;
        struct {
            char* destination;
            char* netmask;
            char* gateway;
            char* metric;
        } route;
    } itf;
} Config;

int read_config_from_json(const char* filename, Config* config) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "open config file failed: %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* json_data = malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    json_data[file_size] = '\0';
    fclose(file);

    cJSON* root = cJSON_Parse(json_data);
    free(json_data);
    
    if (!root) {
        fprintf(stderr, "parse json failed \n");
        return 0;
    }

    // 解析servers部分
    cJSON* servers = cJSON_GetObjectItem(root, "srv");
    if (servers) {
        cJSON* server_ip_json = cJSON_GetObjectItemCaseSensitive(servers, "server_ip");
        cJSON* server_port_json = cJSON_GetObjectItemCaseSensitive(servers, "server_port");
        
        config->srv.server_ip = NULL;
        config->srv.server_port = 0;
        if (server_ip_json && cJSON_IsString(server_ip_json)) {
            config->srv.server_ip = strdup(server_ip_json->valuestring);
        }
        if (server_port_json && cJSON_IsNumber(server_port_json)) {
            config->srv.server_port = server_port_json->valueint;
        }
    }

    // 解析interface部分
    cJSON* itf = cJSON_GetObjectItemCaseSensitive(root, "itf");
    if (itf) {
        cJSON* ifname_json = cJSON_GetObjectItemCaseSensitive(itf, "ifname");
        cJSON* ifdesc_json = cJSON_GetObjectItemCaseSensitive(itf, "ifdesc");
        cJSON* ip_json = cJSON_GetObjectItemCaseSensitive(itf, "ip");
        cJSON* netmask_json = cJSON_GetObjectItemCaseSensitive(itf, "netmask");
        cJSON* mtu_json = cJSON_GetObjectItemCaseSensitive(itf, "mtu");
        cJSON* domain_json = cJSON_GetObjectItemCaseSensitive(itf, "domain");
        cJSON* nameServer_json = cJSON_GetObjectItemCaseSensitive(itf, "nameServer");
        cJSON* searchList_json = cJSON_GetObjectItemCaseSensitive(itf, "searchList");

        if (ifname_json && cJSON_IsString(ifname_json)){
            config->itf.ifname = strdup(ifname_json->valuestring);
        }
        if (ifdesc_json && cJSON_IsString(ifdesc_json)) {
            config->itf.ifdesc = strdup(ifdesc_json->valuestring);
        }
        if (ip_json && cJSON_IsString(ip_json)) {
            config->itf.ip = strdup(ip_json->valuestring);
        }
        if (netmask_json && cJSON_IsString(netmask_json)) {
            config->itf.netmask = strdup(netmask_json->valuestring);
        }
        if (mtu_json && cJSON_IsString(mtu_json)) {
            config->itf.mtu = strdup(mtu_json->valuestring);
        }
        if (domain_json && cJSON_IsString(domain_json)) {
            config->itf.domain = strdup(domain_json->valuestring);
        }
        if (nameServer_json && cJSON_IsString(nameServer_json)) {
            config->itf.nameServer = strdup(nameServer_json->valuestring);
        }
        if (searchList_json && cJSON_IsString(searchList_json)) {
            config->itf.searchList = strdup(searchList_json->valuestring);
        }

        // 解析route
        cJSON* route = cJSON_GetObjectItemCaseSensitive(itf, "route");
        if (route && cJSON_IsObject(route)) {
            cJSON* dest = cJSON_GetObjectItemCaseSensitive(route, "destination");
            cJSON* netmask = cJSON_GetObjectItemCaseSensitive(route, "netmask");
            cJSON* gateway = cJSON_GetObjectItemCaseSensitive(route, "gateway");
            cJSON* metric = cJSON_GetObjectItemCaseSensitive(route, "metric");

            if (dest && cJSON_IsString(dest)) {
                config->itf.route.destination = strdup(dest->valuestring);
            }
            if (netmask && cJSON_IsString(netmask)) {
                config->itf.route.netmask = strdup(netmask->valuestring);
            }
            if (gateway && cJSON_IsString(gateway)) {
                config->itf.route.gateway = strdup(gateway->valuestring);
            }
            if (metric && cJSON_IsString(metric)) {
                config->itf.route.metric = strdup(metric->valuestring);
            }
        }
    }

    cJSON_Delete(root);
    return 1;
}

int main(int argc, char* argv[]) {
    Config config = {0};
    int check = 0;
    if (argc >= 2) {
        char* cmd = argv[1];
        char* config_file = argv[2];
        if (strcmp(cmd, "check") == 0) {
            check = 1;
            if (!read_config_from_json(config_file, &config)) {
                fprintf(stderr, "read config file failed \n");
                return 1;
            }
        }
        else if (strcmp(cmd, "config") == 0) {
            if (!read_config_from_json(config_file, &config)) {
                fprintf(stderr, "read config file failed \n");
                return 1;
            }
        }
    } else {
        fprintf(stderr, "Usage: %s <config.json>\n", argv[0]);
    }

    if (check) {
        printf("srv ip: %s, port: %d\n",
            config.srv.server_ip,
            config.srv.server_port);

        printf("itf: %s, ip: %s, netmask: %s, mtu: %s domain: %s, nameServer: %s, searchList: %s\n",
            config.itf.ifname,
            config.itf.ip,
            config.itf.netmask,
            config.itf.mtu,
            config.itf.domain,
            config.itf.nameServer,
            config.itf.searchList);

        printf("route: %s, netmask: %s, gateway: %s, metric: %s\n",
            config.itf.route.destination,
            config.itf.route.netmask,
            config.itf.route.gateway,
            config.itf.route.metric);
        
        return 0;
    }

    set_reply_server(config.srv.server_ip,
        config.srv.server_port);
    join_network(config.itf.ifname,
        config.itf.ifdesc,
        config.itf.ip,
        config.itf.netmask, 
        config.itf.mtu,
        config.itf.domain, 
        config.itf.nameServer, 
        config.itf.searchList);
    Sleep(3000);
    add_route(config.itf.route.destination,
        config.itf.route.netmask,
        config.itf.route.gateway,
        config.itf.route.metric);

    printf("done \r\n");
    getchar();
    return 0;
}