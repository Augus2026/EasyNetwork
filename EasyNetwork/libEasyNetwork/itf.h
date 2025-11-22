#ifndef ITF_H
#define ITF_H

// 设置ip
BOOL SetInterfaceAddress(const char *interfaceAlias, const char *tunnel_ip, int tunnel_mask);
// 设置网卡MTU
BOOL SetInterfaceMTU(const char* interfaceAlias, DWORD mtu);
// 设置接口DNS
BOOL SetInterfaceDNS(const char* interfaceAlias, char *Domain, char *NameServer, char* SearchList);

// 添加路由
BOOL AddRoute(const char* interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway, DWORD dwForwardMetric1);
// 删除路由
BOOL DeleteRoute(const char *interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway);
// 修改路由
BOOL ModifyRoute(const char* interfaceAlias, const char* destNetwork, const char* netmask, const char* newGateway);
// 清理路由
BOOL CleanRoute(const char* interfaceAlias);

#endif // ITF_H
