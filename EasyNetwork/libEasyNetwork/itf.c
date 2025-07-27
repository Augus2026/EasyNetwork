#include <winsock2.h>
#include <Windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <ip2string.h>
#include <winternl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "wintun.h"
#include <netioapi.h>

#include "wintun_init.h"
#include "tls_client.h"
#include "hdr.h"
#include "itf.h"

NET_IFINDEX GetInterfaceIndex(const wchar_t *interfaceAlias, PMIB_IF_ROW2 row)
{
    NET_IFINDEX ifIndex = 0;
    PMIB_IF_TABLE2 pIfTable = NULL;

    DWORD dwResult = GetIfTable2(&pIfTable);
    if (dwResult != NO_ERROR)
    {
        printf("GetIfTable2 failed with error: %d\n", dwResult);
        goto done;
    }
    for (DWORD i = 0; i < pIfTable->NumEntries; i++)
    {
        // // 将宽字符转换为UTF-8
        // char aliasUtf8[256];
        // WideCharToMultiByte(CP_ACP, 0, pIfTable->Table[i].Alias, -1, aliasUtf8, sizeof(aliasUtf8), NULL, NULL);
        // // 使用printf打印UTF-8字符串
        // printf("InterfaceIndex %d InterfaceAlias %s \r\n", pIfTable->Table[i].InterfaceIndex, aliasUtf8);

        if (lstrcmpW(interfaceAlias, pIfTable->Table[i].Alias) == 0)
        {
            if (row != NULL)
            {
                memcpy(row, &pIfTable->Table[i], sizeof(MIB_IF_ROW2));
            }
            ifIndex = pIfTable->Table[i].InterfaceIndex;
            //break;
        }
    }
    FreeMibTable(pIfTable);

done:
    return ifIndex;
}

VOID PrintIpForwardRow(PMIB_IPFORWARDROW pRow)
{
    struct in_addr addrDest, addrMask, addrNextHop;
    addrDest.s_addr = pRow->dwForwardDest;
    addrMask.s_addr = pRow->dwForwardMask;
    addrNextHop.s_addr = pRow->dwForwardNextHop;

    char s1[INET_ADDRSTRLEN], s2[INET_ADDRSTRLEN], s3[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addrDest, s1, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &addrMask, s2, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &addrNextHop, s3, INET_ADDRSTRLEN);

    printf("dwForwardIfIndex %d \tdwForwardDest %s \tdwForwardMask %s \tdwForwardNextHop %s \r\n", 
        pRow->dwForwardIfIndex, s1, s2, s3);
}

// 设置ip
BOOL _SetInterfaceAddress(NET_IFINDEX ifIndex, DWORD dwForwardDest, UINT8 linkPrefixLength)
{
    MIB_UNICASTIPADDRESS_ROW AddressRow;
    DWORD LastError;

    InitializeUnicastIpAddressEntry(&AddressRow);
    AddressRow.InterfaceIndex = ifIndex;
    AddressRow.Address.Ipv4.sin_family = AF_INET;
    AddressRow.Address.Ipv4.sin_addr.S_un.S_addr = dwForwardDest;
    AddressRow.OnLinkPrefixLength = linkPrefixLength;
    AddressRow.DadState = IpDadStatePreferred;

    LastError = CreateUnicastIpAddressEntry(&AddressRow);
    if (LastError != ERROR_SUCCESS && LastError != ERROR_OBJECT_ALREADY_EXISTS)
    {
        LogError(L"Failed to set IP address", LastError);
        return FALSE;
    }

    return TRUE;
}

BOOL SetInterfaceAddress(const wchar_t *interfaceAlias, const char *tunnel_ip, int tunnel_mask)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        return FALSE;
    }
    DWORD dwForwardDest = inet_addr(tunnel_ip);
    UINT8 linkPrefixLength = tunnel_mask;
    return _SetInterfaceAddress(ifIndex, dwForwardDest, linkPrefixLength);
}

// 设置网卡MTU
BOOL SetInterfaceMTU(const wchar_t *interfaceAlias, DWORD mtu)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        return FALSE;
    }

    // 获取当前接口配置
    MIB_IPINTERFACE_ROW ifRow;
    InitializeIpInterfaceEntry(&ifRow);
    ifRow.Family = AF_INET;
    ifRow.InterfaceIndex = ifIndex;
    DWORD dwResult = GetIpInterfaceEntry(&ifRow);
    if (dwResult != NO_ERROR)
    {
        printf("GetIpInterfaceEntry failed with error: %d\n", dwResult);
		return FALSE;
    }

    // 设置新的MTU值
    ifRow.NlMtu = mtu;
    ifRow.SitePrefixLength = 0;
    dwResult = SetIpInterfaceEntry(&ifRow);
    if (dwResult != NO_ERROR)
    {
        printf("SetIpInterfaceEntry failed with error: %d\n", dwResult);
        return FALSE;
    }

    wprintf(L"Successfully set MTU to %d for interface '%ls'\n", mtu, interfaceAlias);
    return TRUE;
}

BOOL SetInterfaceDNS(const wchar_t *interfaceAlias, wchar_t *Domain, wchar_t *NameServer, wchar_t* SearchList)
{
    // 获取网络接口索引
    MIB_IF_ROW2 row;
    DWORD ifIndex = GetInterfaceIndex(interfaceAlias, &row);
    if (ifIndex == 0)
    {
        return FALSE;
    }

    // 准备DNS设置
    DNS_INTERFACE_SETTINGS dnsSettings;
    ZeroMemory(&dnsSettings, sizeof(dnsSettings));
    dnsSettings.Version = DNS_INTERFACE_SETTINGS_VERSION1;
    dnsSettings.Flags = DNS_SETTING_DOMAIN | DNS_SETTING_NAMESERVER | DNS_SETTING_SEARCHLIST;
    dnsSettings.Domain = Domain;
    dnsSettings.NameServer = NameServer;
    dnsSettings.SearchList = SearchList;

    // 设置DNS
    DWORD result = SetInterfaceDnsSettings(row.InterfaceGuid, &dnsSettings);
    if (result != NO_ERROR)
    {
        wprintf(L"SetInterfaceDnsSettings failed with error: %d\n", result);
        return FALSE;
    }

    wprintf(L"Successfully set DNS for interface '%s'\n", interfaceAlias);
    return TRUE;
}

// 添加路由
BOOL _AddRoute(NET_IFINDEX ifIndex, DWORD dwForwardDest, DWORD dwForwardMask, DWORD dwForwardNextHop, DWORD dwForwardMetric1)
{
    // Declare and initialize variables
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    PMIB_IPFORWARDROW pRow = NULL;
    DWORD dwSize = 0;
    BOOL bOrder = TRUE;
    DWORD dwStatus = 0;

    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        // Allocate the memory for the table
        if (!(pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize)))
        {
            printf("Malloc failed. Out of memory.\n");
            return FALSE;
        }
        // Now get the table.
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }
    if (dwStatus != ERROR_SUCCESS)
    {
        printf("GetIpForwardTable failed with error: %d\n", dwStatus);
        if (pIpForwardTable)
            free(pIpForwardTable);
        return FALSE;
    }

    for (int i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
        // 有默认路由情况下才生效，现在没有默认路由
        if (pIpForwardTable->table[i].dwForwardDest == 0)
        {
            if (!pRow)
            {
                // Allocate some memory to store the row in; this is easier than filling
                // in the row structure ourselves, and we can be sure we change only the
                // gateway address.
                pRow = (PMIB_IPFORWARDROW)malloc(sizeof(MIB_IPFORWARDROW));
                if (!pRow) {
                    printf("Malloc failed. Out of memory.\n");
                    exit(1);
                }
                // Copy the row
                memcpy(pRow, &(pIpForwardTable->table[i]), sizeof(MIB_IPFORWARDROW));
            }
        }
    }

    if (pRow == NULL) {
        printf("default route is empty.");
        if (pIpForwardTable)
            free(pIpForwardTable);
        return FALSE;
    }

    pRow->dwForwardIfIndex = ifIndex;
    pRow->dwForwardDest = dwForwardDest;
    pRow->dwForwardMask = dwForwardMask;
    pRow->dwForwardNextHop = dwForwardNextHop;
    pRow->dwForwardMetric1 = dwForwardMetric1;

    // Create a new route entry.
    dwStatus = CreateIpForwardEntry(pRow);
    if (dwStatus == NO_ERROR)
        printf("Gateway changed successfully\n");
    else if (dwStatus == ERROR_INVALID_PARAMETER)
        printf("Invalid parameter.\n");
    else
        printf("Error: %d\n", dwStatus);

    // Free resources
    if (pIpForwardTable)
        free(pIpForwardTable);
    if (pRow)
        free(pRow);

    return TRUE;
}

BOOL AddRoute(const wchar_t *interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway, DWORD dwForwardMetric1)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        wprintf("Interface not found: %ls\n", interfaceAlias);
        return FALSE;
    }
    DWORD dwForwardDest = inet_addr(destNetwork);
    DWORD dwForwardMask = inet_addr(netmask);
    DWORD dwForwardNextHop = inet_addr(gateway);
    return _AddRoute(ifIndex, dwForwardDest, dwForwardMask, dwForwardNextHop, dwForwardMetric1);
}

// 删除路由
BOOL _DeleteRoute(NET_IFINDEX ifIndex, DWORD dwForwardDest, DWORD dwForwardMask, DWORD dwForwardNextHop)
{
    // 获取路由表
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    DWORD dwSize = 0;
    BOOL bOrder = TRUE;
    DWORD dwStatus;
    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
        if (pIpForwardTable == NULL)
        {
            printf("Memory allocation failed\n");
            return FALSE;
        }
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }
    if (dwStatus != NO_ERROR)
    {
        printf("GetIpForwardTable failed with error: %d\n", dwStatus);
        if (pIpForwardTable)
            free(pIpForwardTable);
        return FALSE;
    }

    // 查找并删除匹配的路由
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
        if (pIpForwardTable->table[i].dwForwardIfIndex == ifIndex &&
            pIpForwardTable->table[i].dwForwardDest == dwForwardDest &&
            pIpForwardTable->table[i].dwForwardMask == dwForwardMask &&
            pIpForwardTable->table[i].dwForwardNextHop == dwForwardNextHop)
        {
            dwStatus = DeleteIpForwardEntry(&(pIpForwardTable->table[i]));
            if (dwStatus != ERROR_SUCCESS) {
                printf("Could not delete old gateway\n");
                exit(1);
            }
        }
    }

    // Free resources
    if (pIpForwardTable)
        free(pIpForwardTable);

    return TRUE;
}

BOOL DeleteRoute(const wchar_t *interfaceAlias, const char *destNetwork, const char *netmask, const char *gateway)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        wprintf("Interface not found: %ls\n", interfaceAlias);
        return FALSE;
    }
    DWORD dwForwardDest = inet_addr(destNetwork);
    DWORD dwForwardMask = inet_addr(netmask);
    DWORD dwForwardNextHop = inet_addr(gateway);
    return _DeleteRoute(ifIndex, dwForwardDest, dwForwardMask, dwForwardNextHop);
}

// 修改路由
BOOL _ModifyRoute(NET_IFINDEX ifIndex, DWORD dwForwardDest, DWORD dwForwardMask, DWORD dwForwardNextHop)
{
    // 获取路由表
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    PMIB_IPFORWARDROW pRow = NULL;
    DWORD dwSize = 0;
    BOOL bOrder = TRUE;
    DWORD dwStatus = 0;

    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
        if (pIpForwardTable == NULL)
        {
            printf("Memory allocation failed\n");
            return FALSE;
        }
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }
    if (dwStatus != NO_ERROR)
    {
        printf("GetIpForwardTable failed with error: %d\n", dwStatus);
        free(pIpForwardTable);
        return FALSE;
    }

    // 查找并修改匹配的路由
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
        if (pIpForwardTable->table[i].dwForwardIfIndex == ifIndex &&
            pIpForwardTable->table[i].dwForwardDest == dwForwardDest &&
            pIpForwardTable->table[i].dwForwardMask == dwForwardMask) {
            // We have found the default gateway.
            if (!pRow) {
                // Allocate some memory to store the row in. This is easier than filling
                // in the row structure ourselves, and we can be sure to change only the
                // gateway address.
                pRow = (PMIB_IPFORWARDROW) malloc(sizeof(MIB_IPFORWARDROW));
                if (!pRow) {
                    printf("Malloc failed. Out of memory.\n");
                    exit(1);
                }
                // Copy the row.
                memcpy(pRow, &(pIpForwardTable->table[i]), sizeof(MIB_IPFORWARDROW));
            }

            // Delete the old default gateway entry.
            dwStatus = DeleteIpForwardEntry(&(pIpForwardTable->table[i]));
            if (dwStatus != ERROR_SUCCESS) {
                printf("Could not delete old gateway\n");
                exit(1);
            }
        }
    }

    pRow->dwForwardNextHop = dwForwardNextHop;

    dwStatus = CreateIpForwardEntry(pRow);
    if (dwStatus == NO_ERROR)
        printf("Gateway changed successfully\n");
    else if (dwStatus == ERROR_INVALID_PARAMETER)
        printf("Invalid parameter.\n");
    else
        printf("Error: %d\n", dwStatus);

    // Free resources.
    if (pIpForwardTable)
        free(pIpForwardTable);
    if (pRow)
        free(pRow);

    return TRUE;
}

BOOL ModifyRoute(const wchar_t* interfaceAlias, const char* destNetwork, const char* netmask, const char* newGateway)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        wprintf("Interface not found: %ls\n", interfaceAlias);
        return FALSE;
    }
    DWORD dwForwardDest = inet_addr(destNetwork);
    DWORD dwForwardMask = inet_addr(netmask);
    DWORD dwForwardNextHop = inet_addr(newGateway);
    return _ModifyRoute(ifIndex, dwForwardDest, dwForwardMask, dwForwardNextHop);
}

// 清理路由
BOOL _CleanRoute(NET_IFINDEX ifIndex)
{
    // 获取路由表
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    DWORD dwSize = 0;
    BOOL bOrder = TRUE;
    DWORD dwStatus;
    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER)
    {
        pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize);
        if (pIpForwardTable == NULL)
        {
            printf("Memory allocation failed\n");
            return FALSE;
        }
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }
    if (dwStatus != NO_ERROR)
    {
        printf("GetIpForwardTable failed with error: %d\n", dwStatus);
        if (pIpForwardTable)
            free(pIpForwardTable);
        return FALSE;
    }

    // 查找并删除匹配的路由
    for (DWORD i = 0; i < pIpForwardTable->dwNumEntries; i++)
    {
        PMIB_IPFORWARDROW pRoute = &pIpForwardTable->table[i];
        if (pRoute->dwForwardIfIndex == ifIndex && pRoute->ForwardProto != RouteProtocolLocal)
        {
            dwStatus = DeleteIpForwardEntry(pRoute);
            if (dwStatus != ERROR_SUCCESS) {
                printf("Could not delete old gateway\n");
                exit(1);
            }
        }
    }

    // Free resources
    if (pIpForwardTable)
        free(pIpForwardTable);

    return TRUE;
}

// 清理路由
BOOL CleanRoute(const wchar_t* interfaceAlias)
{
    NET_IFINDEX ifIndex = GetInterfaceIndex(interfaceAlias, NULL);
    if (ifIndex == 0)
    {
        wprintf("Interface not found: %ls\n", interfaceAlias);
        return FALSE;
    }
    return _CleanRoute(ifIndex);
}