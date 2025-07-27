/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018-2021 WireGuard LLC. All Rights Reserved.
 */

#include "wintun_peer.h"

#include "message.h"
#include "kcp_client.h"
#include "udp_client.h"
#include "ikcp.h"

#include <windows.h>
#pragma comment(lib, "winmm.lib")

static HANDLE QuitEvent;
static volatile BOOL HaveQuit;
static volatile BOOL CloseAppQuit;

static inline void update_peer_kcp(peer_info_t* peer) {
    IUINT32 current_clock = iclock();
    IUINT32 next_update = ikcp_check(peer->kcp, current_clock);
    if (current_clock >= next_update) {
        ikcp_update(peer->kcp, current_clock);
    }
}

void send_thread(void* arg) {
    peer_info_t* peer = (peer_info_t*)arg;

    kcp_client_new(peer);

    fd_set readfds;
	struct timeval timeout;

    while (1) {
        update_peer_kcp(peer);

        EnterCriticalSection(&peer->snd_queue_cs);
        while (!iqueue_is_empty(&peer->snd_queue)) {
			MsgNode* node = iqueue_entry(peer->snd_queue.next, MsgNode, head);
            IUINT32 clock = node->timestamp;
            int index = node->index;

            ikcp_send(peer->kcp, node->msg, node->msglen);
            ikcp_flush(peer->kcp);

            IUINT32 diff = iclock() - clock;
            if(index != 1) {
                printf("[wintun -> net] send packet: %dms \r\n", diff);
                print_hex(node->msg->data, node->msg->head.size);
            }

            iqueue_del(&node->head);
			free(node);
            free(node->msg);
		}
        LeaveCriticalSection(&peer->snd_queue_cs);

        timeBeginPeriod(1);
        FD_ZERO(&readfds);
        FD_SET(peer->sockfd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        int activity = select(peer->sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == -1) {
            perror("select error");
        } else if (activity == 0) {
            unsigned int clock2 = iclock();
            // printf("client once: %dms \r\n", clock2 - clock1);
            continue;
        } else {
            kcp_recv_once(peer);
        }
        timeEndPeriod(1);
    }
    kcp_client_close(peer);

    free(peer);
}

void alive_thread(void* arg) {
    peer_info_t* peer = (peer_info_t*)arg;
    srand(time(0));
    peer->code = rand() % 1000;
    while (1) {
        MsgNode* node = malloc(sizeof(MsgNode));
        node->msg = new_kcp_ping(peer);
        node->msglen = sizeof(message_head_t) + node->msg->head.size;
        node->timestamp = iclock();
        node->index = 1;
        
        EnterCriticalSection(&peer->snd_queue_cs);
        iqueue_add_tail(&node->head, &peer->snd_queue);
        LeaveCriticalSection(&peer->snd_queue_cs);

        Sleep(3000);
    }
}

void kcp_send_thread(peer_info_t* peer) {
    HANDLE send_thread_handle;
    DWORD send_thread_id;
    send_thread_handle = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)send_thread,
        (VOID*)peer,
        0,
        &send_thread_id
    );

#if 1
    HANDLE alive_thread_handle;
    DWORD alive_thread_id;
    alive_thread_handle = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)alive_thread,
        (VOID*)peer,
        0,
        &alive_thread_id
    );
#endif
    
    // 添加注册消息
    MsgNode* node = malloc(sizeof(MsgNode));
    node->msg = new_kcp_register_peer(peer);
    node->msglen = sizeof(message_head_t) + node->msg->head.size;
    
    EnterCriticalSection(&peer->snd_queue_cs);
    iqueue_add_tail(&node->head, &peer->snd_queue);
    LeaveCriticalSection(&peer->snd_queue_cs);

    return 0;
}

static DWORD WINAPI
KcpReceivePackets(_Inout_ LPVOID lpParam)
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
            // print_hex(buffer, len);

#if 0
            package_data_t* package = (package_data_t*)malloc(sizeof(package_data_t) + len);
            memset(package, 0, sizeof(package_data_t));
            package->head.magic = 0x1234;
            package->head.msgtype = TRANSPORT_DATA;
            // 使用ip结构体指向IP包头
            ip_hdr* ip4_hdr = (ip_hdr*)buffer;
            if (len >= 20) {
                memcpy(&package->head.src_addr, &ip4_hdr->srcaddr, sizeof(ip4_hdr->srcaddr));
                memcpy(&package->head.dst_addr, &ip4_hdr->dstaddr, sizeof(ip4_hdr->dstaddr));
            } else {
                printf("Invalid IP packet header length\n");
                free(package);
                return -1;
            }
            package->size = len;
            memcpy(package->data, buffer, len);
            int ret = wolfSSL_write(peer->ssl, package, sizeof(package_data_t) + len);
            if (ret <= 0) {
                printf("Send data failed, error: %d\n", wolfSSL_get_error(peer->ssl, ret));               

                free(package);
                WintunReleaseReceivePacket(Session, Packet);
                return -1;
            }
            free(package);
#else
            // 网卡接收到数据，通过kcp发送数据
            MsgNode* node = malloc(sizeof(MsgNode));
            node->msg = new_kcp_transport_data(peer, Packet, PacketSize);
            node->msglen = sizeof(message_head_t) + node->msg->head.size;
            node->timestamp = iclock();
            node->index = 0;

            EnterCriticalSection(&peer->snd_queue_cs);
            iqueue_add_tail(&node->head, &peer->snd_queue);
            LeaveCriticalSection(&peer->snd_queue_cs);
#endif
            
            WintunReleaseReceivePacket(Session, Packet);
        }
        else
        {
#if 0
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
                        MY_SSL_Cleanup(peer);
                    }
                    continue;
                }
                return ERROR_SUCCESS;
            default:
                LogError(L"Packet read failed", LastError);
                return LastError;
            }
#endif
            Sleep(1);
        }
    }
    // printf("ReceivePackets done\n");
    return ERROR_SUCCESS;
}

static DWORD WINAPI
KcpSendPackets(_Inout_ LPVOID lpParam)
{
    // 调置线程亲和性
    DWORD_PTR affinity = 2;
    SetThreadAffinityMask(GetCurrentThread(), affinity);

    peer_info_t* peer = (peer_info_t*)lpParam;
    // package_data_t* package = (package_data_t*)malloc(sizeof(package_data_t));
    WINTUN_SESSION_HANDLE Session = peer->Session;
    while (!HaveQuit)
    {
#if 0
        int retval;
        // 读取数据头
        retval = read_peer_data(peer->ssl, (char*)package, sizeof(package_data_t));
        if(retval < 0)
        {
            printf("read package header error.\r\n");
            break;
        }
        //print_hex(package, sizeof(package_data_t));
        // 读取数据
        package = (package_data_t*)realloc(package, sizeof(package_data_t) + package->size);
        retval = read_peer_data(peer->ssl, package->data, package->size);
        if(retval < 0)
        {
            printf("read package data error.\r\n");
            break;
        }
        // print_hex(package->data, package->size);
        // 读取数据
        BYTE *Packet = WintunAllocateSendPacket(Session, package->size);
        if (Packet)
        {
            memcpy(Packet, package->data, package->size);
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
#else
        EnterCriticalSection(&peer->rcv_queue_cs);
        // kcp接收到数据后，通过wintun发送数据
        while (!iqueue_is_empty(&peer->rcv_queue)) {
            MsgNode* node = iqueue_entry(peer->rcv_queue.next, MsgNode, head);
            UINT32 clock = node->timestamp;

            message_head_t* msg = node->msg;
            iqueue_del(&node->head);
            free(node);

            // 读取数据
            BYTE *Packet = WintunAllocateSendPacket(Session, msg->head.size);
            if (Packet) {
               memcpy(Packet, msg->data, msg->head.size);
               WintunSendPacket(Session, Packet);
            } else if (GetLastError() != ERROR_BUFFER_OVERFLOW)
               return LogLastError(L"Packet write failed");
            

            UINT32 diff = iclock() - clock;
            printf("[net -> wintun] recv packet: %dms \n", diff);
            print_hex(msg->data, msg->head.size);
            free(msg);
        }
        LeaveCriticalSection(&peer->rcv_queue_cs);
#endif
    }
    // free(package);
    // printf("SendPackets done\n");
    return ERROR_SUCCESS;
}

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
            int ret = wolfSSL_write(peer->ssl, message, sizeof(message_head_t) + len);
            if (ret <= 0) {
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
                        MY_SSL_Cleanup(peer);
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
        retval = read_peer_data(peer->ssl, (char*)message, sizeof(message_head_t));
        if(retval < 0) {
            printf("read package header error.\r\n");
            break;
        }
        // 读取数据
        message = (message_head_t*)realloc(message, sizeof(message_head_t) + message->head.size);
        retval = read_peer_data(peer->ssl, message->data, message->head.size);
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
        wprintf(L"Failed to set DNS\n");
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
#if 1
        if(peer->ssl == NULL) {
            MY_SSL_Init(peer);
            if(peer->ssl == NULL) {
                printf("MY_SSL_Init failed \r\n");
                goto done;
            }
        } else {
            MY_SSL_Reconnect(peer);
            if (peer->ssl == NULL)
            {
                printf("MY_SSL_Reconnect failed \r\n");
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
#else
        // 初始化发送，接收队列
        iqueue_init(&peer->snd_queue);
        iqueue_init(&peer->rcv_queue);

        // 初始化队列锁
        InitializeCriticalSection(&peer->snd_queue_cs);
        InitializeCriticalSection(&peer->rcv_queue_cs);

        kcp_send_thread(peer);
        Sleep(3000);

        // 转发数据
        HANDLE Workers[] = { CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KcpReceivePackets, (LPVOID)peer, 0, NULL),
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KcpSendPackets, (LPVOID)peer, 0, NULL) };
        if (!Workers[0] || !Workers[1])
        {
            LastError = LogError(L"Failed to create threads", GetLastError());
            goto cleanupWorkers;
        }
#endif

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
            printf("CloseAppQuit\r\n");
            break;
        }

done:
        int secs = 0x01 << log_n;
        printf("Sleep %ds.. \r\n", secs);
        Sleep(secs * 1000);
        ++log_n;
        if(log_n > 10) {
            log_n = 10;
        }
        printf("Reconnecting...\r\n");
    }
    LastError = ERROR_SUCCESS;

cleanupWorkers:
    HaveQuit = TRUE;
    SetEvent(QuitEvent);
    //for (size_t i = 0; i < _countof(Workers); ++i)
    //{
    //    if (Workers[i])
    //    {
    //        WaitForSingleObject(Workers[i], INFINITE);
    //        CloseHandle(Workers[i]);
    //    }
    //}
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