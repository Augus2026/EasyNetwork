/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018-2021 WireGuard LLC. All Rights Reserved.
 */


#include "wintun_peer.h"
#include "message.h"
#include <windows.h>

static HANDLE QuitEvent;
static volatile BOOL HaveQuit;
static volatile BOOL CloseAppQuit;

static void
PrintPacket(_In_ const BYTE *Packet, _In_ DWORD PacketSize)
{
    if (PacketSize < 20)
    {
        Log(WINTUN_LOG_INFO, L"Received packet without room for an IP header");
        return;
    }
    BYTE IpVersion = Packet[0] >> 4, Proto;
    WCHAR Src[46], Dst[46];
    if (IpVersion == 4)
    {
        RtlIpv4AddressToStringW((struct in_addr *)&Packet[12], Src);
        RtlIpv4AddressToStringW((struct in_addr *)&Packet[16], Dst);
        Proto = Packet[9];
        Packet += 20, PacketSize -= 20;
    }
    else if (IpVersion == 6 && PacketSize < 40)
    {
        Log(WINTUN_LOG_INFO, L"Received packet without room for an IP header");
        return;
    }
    else if (IpVersion == 6)
    {
        RtlIpv6AddressToStringW((struct in6_addr *)&Packet[8], Src);
        RtlIpv6AddressToStringW((struct in6_addr *)&Packet[24], Dst);
        Proto = Packet[6];
        Packet += 40, PacketSize -= 40;
    }
    else
    {
        Log(WINTUN_LOG_INFO, L"Received packet that was not IP");
        return;
    }
    if (Proto == 1 && PacketSize >= 8 && Packet[0] == 0)
        Log(WINTUN_LOG_INFO, L"Received IPv%d ICMP echo reply from %s to %s", IpVersion, Src, Dst);
    else
        Log(WINTUN_LOG_INFO, L"Received IPv%d proto 0x%x packet from %s to %s", IpVersion, Proto, Src, Dst);
}

static void
RegisterPeer(peer_info_t* peer) {
    message_head_t* package = (message_head_t*)malloc(sizeof(message_head_t));
    memset(package, 0, sizeof(message_head_t));
    package->head.magic = 0x1234;
    package->head.msgtype = REGISTER_PEER;
    package->head.register_addr.s_addr = inet_addr(peer->tunnel_ip);
    package->head.size = 0;

    int ret = wolfSSL_write(peer->ssl, package, sizeof(message_head_t));
    if (ret <= 0) {
        printf("Send data failed, error: %d\n", wolfSSL_get_error(peer->ssl, ret));               
    }
    free(package);
}

static DWORD WINAPI
ReceivePackets(_Inout_ LPVOID lpParam)
{
    // 调置线程亲和性
    DWORD_PTR affinity = 1;
    SetThreadAffinityMask(GetCurrentThread(), affinity);

    peer_info_t* peer = (peer_info_t*)lpParam;
    WINTUN_SESSION_HANDLE Session = peer->Session;
    HANDLE WaitHandles[] = { WintunGetReadWaitEvent(Session), QuitEvent };
    while (!HaveQuit)
    {
        DWORD PacketSize;
        BYTE *Packet = WintunReceivePacket(Session, &PacketSize);
        if (Packet)
        {
            // PrintPacket(Packet, PacketSize);
            int len = PacketSize;
            char* buffer = (char*)Packet;
            // 检查是否为IPv6数据包
            if (len >= 1) {
                unsigned char version = ((unsigned char*)buffer)[0] >> 4;
                if (version == 6) {
                    // printf("IPv6 packet detected, skipping processing\n");
                    WintunReleaseReceivePacket(Session, Packet);
                    continue;
                }
            }
            message_head_t* message = (message_head_t*)malloc(sizeof(message_head_t) + len);
            memset(message, 0, sizeof(message_head_t));
            message->head.magic = 0x1234;
            message->head.msgtype = TRANSPORT_DATA;
            // 使用ip结构体指向IP包头
            ip_hdr* ip4_hdr = (ip_hdr*)buffer;
            if (len >= 20) {
                memcpy(&message->head.src_addr, &ip4_hdr->srcaddr, sizeof(ip4_hdr->srcaddr));
                memcpy(&message->head.dst_addr, &ip4_hdr->dstaddr, sizeof(ip4_hdr->dstaddr));
            } else {
                printf("Invalid IP packet header length\n");
                free(message);
                return -1;
            }
            message->head.size = len;
            memcpy(message->data, buffer, len);
            int ret = ssl_write_data(peer, message, sizeof(message_head_t) + len);
            if (ret < 0) {
                printf("Send data failed, error: %d\n", wolfSSL_get_error(peer->ssl, ret));
                free(message);
                WintunReleaseReceivePacket(Session, Packet);
                return -1;
            }
            free(message);
            WintunReleaseReceivePacket(Session, Packet);
        }
        else
        {
            DWORD LastError = GetLastError();
            switch (LastError)
            {
            case ERROR_NO_MORE_ITEMS:
                DWORD result = WaitForMultipleObjects(_countof(WaitHandles), WaitHandles, FALSE, INFINITE);
                if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + _countof(WaitHandles)) {
                    int index = result - WAIT_OBJECT_0;
                    if (index == 0) {
                        // printf("Wintun read event triggered\n");
                    } else if (index == 1) {
                        // printf("QuitEvent triggered\n");
                        ssl_cleanup(peer);
                    }
                    continue;
                }
                return ERROR_SUCCESS;
            default:
                LogError(L"Packet read failed", LastError);
                return LastError;
            }
        }
    }
    // printf("ReceivePackets done\n");
    return ERROR_SUCCESS;
}

static DWORD WINAPI
SendPackets(_Inout_ LPVOID lpParam)
{
    // 调置线程亲和性
    DWORD_PTR affinity = 2;
    SetThreadAffinityMask(GetCurrentThread(), affinity);

    peer_info_t* peer = (peer_info_t*)lpParam;
    message_head_t* message = (message_head_t*)malloc(sizeof(message_head_t));
    WINTUN_SESSION_HANDLE Session = peer->Session;
    while (!HaveQuit) {
        int retval;
        // 读取数据头
        retval = ssl_read_data(peer, (char*)message, sizeof(message_head_t));
        if(retval < 0) {
            printf("read package header error.\r\n");
            break;
        }
        // 读取数据
        message = (message_head_t*)realloc(message, sizeof(message_head_t) + message->head.size);
        retval = ssl_read_data(peer, message->data, message->head.size);
        if(retval < 0) {
            printf("read package data error.\r\n");
            break;
        }
        // 读取数据
        BYTE *Packet = WintunAllocateSendPacket(Session, message->head.size);
        if (Packet)
        {
            memcpy(Packet, message->data, message->head.size);
            WintunSendPacket(Session, Packet);
        }
        else if (GetLastError() != ERROR_BUFFER_OVERFLOW)
            return LogLastError(L"Packet write failed");

        // switch (WaitForSingleObject(QuitEvent, 1000 /* 1 second */))
        // {
        // case WAIT_ABANDONED:
        // case WAIT_OBJECT_0:
        //     return ERROR_SUCCESS;
        // }
    }
    free(message);
    // printf("SendPackets done\n");
    return ERROR_SUCCESS;
}

// 添加辅助函数
static wchar_t* char_to_wchar(const char* str) {
    if (!str) return NULL;
    
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (len <= 0) return NULL;
    
    wchar_t* wstr = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!wstr) return NULL;
    
    if (MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len) == 0) {
        free(wstr);
        return NULL;
    }
    
    return wstr;
}

void init_tunnel(peer_info_t* peer) {
    // 设置接口地址
    SetInterfaceAddress(peer->name, peer->tunnel_ip, peer->tunnel_mask);
    // 设置MTU
    int _mtu = 1400;
    SetInterfaceMTU(peer->name, peer->mtu);
    
    // 设置DNS
    wchar_t* Domain = char_to_wchar(peer->domain);
    wchar_t* NameServer = char_to_wchar(peer->name_server); 
    wchar_t* SearchList = char_to_wchar(peer->search_list);
    BOOL dnsResult = SetInterfaceDNS(peer->name, 
        Domain ? Domain : L"", 
        NameServer ? NameServer : L"", 
        SearchList ? SearchList : L"");
    if (Domain) free(Domain);
    if (NameServer) free(NameServer);
    if (SearchList) free(SearchList);

    if (!dnsResult) {
        LOG_ERR(L"Failed to set DNS");
    }
}

// 初始化peer
int build_peer(peer_info_t* peer)
{
    HMODULE Wintun = InitializeWintun();
    if (!Wintun)
        return LogError(L"Failed to initialize Wintun", GetLastError());
    WintunSetLogger(ConsoleLogger);
    Log(WINTUN_LOG_INFO, L"Wintun library loaded");

    DWORD LastError;
    HaveQuit = FALSE;
    CloseAppQuit = FALSE;
    QuitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!QuitEvent)
    {
        LastError = LogError(L"Failed to create event", GetLastError());
        goto cleanupWintun;
    }

    GUID InterfaceGuid;
    UuidCreate(&InterfaceGuid);
    WINTUN_ADAPTER_HANDLE Adapter = WintunCreateAdapter(peer->name, peer->desc, &InterfaceGuid);
    if (!Adapter)
    {
        LastError = GetLastError();
        LogError(L"Failed to create adapter", LastError);
        goto cleanupQuit;
    }

    DWORD Version = WintunGetRunningDriverVersion();
    Log(WINTUN_LOG_INFO, L"Wintun v%u.%u loaded", (Version >> 16) & 0xff, (Version >> 0) & 0xff);

    WINTUN_SESSION_HANDLE Session = WintunStartSession(Adapter, 0x400000);
    if (!Session)
    {
        LastError = LogLastError(L"Failed to create adapter");
        goto cleanupAdapter;
    }
    peer->Session = Session;
    Log(WINTUN_LOG_INFO, L"Launching threads and mangling packets...");

    init_tunnel(peer);

    int log_n = 0;
    while(1) {
        HaveQuit = FALSE;
        ResetEvent(QuitEvent);

        if(peer->ssl == NULL) {
            ssl_init(peer);
            if(peer->ssl == NULL) {
                LOG_ERR(L"ssl_init failed");
                goto done;
            }
        } else {
            ssl_reconnect(peer);
            if (peer->ssl == NULL) {
                LOG_ERR(L"ssl_reconnect failed");
                goto done;
            }
        }
        // 注册隧道
        RegisterPeer(peer);
        // 转发数据
        HANDLE Workers[] = { CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceivePackets, (LPVOID)peer, 0, NULL),
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendPackets, (LPVOID)peer, 0, NULL) };
        if (!Workers[0] || !Workers[1])
        {
            LastError = LogError(L"Failed to create threads", GetLastError());
            goto cleanupWorkers;
        }

        // 先等待SendPackets线程退出
        WaitForSingleObject(Workers[1], INFINITE);
        CloseHandle(Workers[1]);
        // 触发退出事件
        HaveQuit = TRUE;
        SetEvent(QuitEvent);
        // 再等待ReceivePackets线程退出
        WaitForSingleObject(Workers[0], INFINITE);
        CloseHandle(Workers[0]);

        if(CloseAppQuit) {
            LOG_INFO(L"CloseAppQuit");
            break;
        }

done:
        int secs = 0x01 << log_n;
        LOG_INFO(L"Sleep %ds..", secs);
        Sleep(secs * 1000);
        ++log_n;
        if(log_n > 10) {
            log_n = 10;
        }
        LOG_INFO(L"Reconnecting...");
    }
    LastError = ERROR_SUCCESS;

cleanupWorkers:
    HaveQuit = TRUE;
    SetEvent(QuitEvent);
#if 0
    for (size_t i = 0; i < _countof(Workers); ++i)
    {
       if (Workers[i])
       {
           WaitForSingleObject(Workers[i], INFINITE);
           CloseHandle(Workers[i]);
       }
    }
#endif
    WintunEndSession(Session);
cleanupAdapter:
    WintunCloseAdapter(Adapter);
cleanupQuit:
    CloseHandle(QuitEvent);
cleanupWintun:
    FreeLibrary(Wintun);
    return LastError;
}

// 销毁peer
void destroy_peer()
{
    Log(WINTUN_LOG_INFO, L"Cleaning up and shutting down...");
    HaveQuit = TRUE;
    SetEvent(QuitEvent);
    CloseAppQuit = TRUE;
    return TRUE;
}