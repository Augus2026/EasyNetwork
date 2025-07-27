#ifndef ITF_H
#define ITF_H

// 设置ip
BOOL SetInterfaceAddress(const wchar_t *interfaceAlias, const char *tunnel_ip, int tunnel_mask);
// 设置网卡MTU
BOOL SetInterfaceMTU(const wchar_t* interfaceAlias, DWORD mtu);
// 设置接口DNS
BOOL SetInterfaceDNS(const wchar_t *interfaceAlias, wchar_t *Domain, wchar_t *NameServer, wchar_t* SearchList);

// 添加路由
BOOL AddRoute(const wchar_t *interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway, DWORD dwForwardMetric1);
// 删除路由
BOOL DeleteRoute(const wchar_t *interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway);
// 修改路由
BOOL ModifyRoute(const wchar_t* interfaceAlias, const char* destNetwork, const char* netmask, const char* newGateway);
// 清理路由
BOOL CleanRoute(const wchar_t* interfaceAlias);

#endif
