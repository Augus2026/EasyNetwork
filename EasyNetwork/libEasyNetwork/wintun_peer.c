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

static DWORD WINAPI
ReceivePackets(_Inout_ LPVOID lpParam) {
    peer_info_t* peer = (peer_info_t*)lpParam;

    // set thread affinity
    DWORD_PTR affinity = 1;
    SetThreadAffinityMask(GetCurrentThread(), affinity);
    
    // loop read message and send packet
    WINTUN_SESSION_HANDLE Session = peer->Session;
    HANDLE WaitHandles[] = { WintunGetReadWaitEvent(Session), QuitEvent };
    while (!HaveQuit) {
        DWORD PacketSize;
        BYTE *Packet = WintunReceivePacket(Session, &PacketSize);
        if (Packet) {
            PrintPacket(Packet, PacketSize);

            // check if it is an IPv6 packet
            if (PacketSize >= 1) {
                unsigned char Version = ((unsigned char*)Packet)[0] >> 4;
                if (Version == 6) {
                    printf("IPv6 packet detected, skipping processing\n");
                    WintunReleaseReceivePacket(Session, Packet);
                    continue;
                }
            }

            // send message
            if(send_message(peer, Packet, PacketSize) != ERROR_SUCCESS) {
                LogError(L"Send message failed", GetLastError());
                continue;
            }

            // release packet
            WintunReleaseReceivePacket(Session, Packet);
        } else {
            DWORD LastError = GetLastError();
            switch (LastError) {
            case ERROR_NO_MORE_ITEMS:
                DWORD result = WaitForMultipleObjects(_countof(WaitHandles), WaitHandles, FALSE, INFINITE);
                if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + _countof(WaitHandles)) {
                    int index = result - WAIT_OBJECT_0;
                    if (index == 0) {
                    } else if (index == 1) {
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

    return ERROR_SUCCESS;
}

static DWORD WINAPI
SendPackets(_Inout_ LPVOID lpParam) {
    peer_info_t* peer = (peer_info_t*)lpParam;

    // set thread affinity
    DWORD_PTR affinity = 2;
    SetThreadAffinityMask(GetCurrentThread(), affinity);

    // loop read message and send packet
    WINTUN_SESSION_HANDLE Session = peer->Session;
    while (!HaveQuit) {
        // read message
        message_head_t* msg = recv_message(peer);
        if (!msg) {
            continue;
        }

        // send packet
        BYTE *Packet = WintunAllocateSendPacket(Session, msg->head.size);
        if (Packet) {
            memcpy(Packet, msg->data, msg->head.size);
            WintunSendPacket(Session, Packet);
        } else if (GetLastError() != ERROR_BUFFER_OVERFLOW) {
            return LogLastError(L"Packet write failed");
        } 

        // check quit event
        switch (WaitForSingleObject(QuitEvent, 0)) {
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0:
            return ERROR_SUCCESS;
        }
    }

    return ERROR_SUCCESS;
}

void init_peer(peer_info_t* peer) {
    BOOL Result = FALSE;

    // 设置接口地址
    Result = SetInterfaceAddress(peer->name,
        peer->tunnel_ip,
        peer->tunnel_mask);
    if (!Result) {
        LogError(L"Failed to set interface address", GetLastError());
    }
    
    // 设置MTU
    Result = SetInterfaceMTU(peer->name,
        peer->mtu);
    if (!Result) {
        LogError(L"Failed to set interface MTU", GetLastError());
    }
    
    // 设置DNS
    Result = SetInterfaceDNS(peer->name, 
        peer->domain,
        peer->name_server,
        peer->search_list);
    if (!Result) {
        LogError(L"Failed to set interface DNS", GetLastError());
    }
}

int build_peer(peer_info_t* peer) {
    HMODULE Wintun = InitializeWintun();
    if (!Wintun)
        return LogError(L"Failed to initialize Wintun", GetLastError());
    WintunSetLogger(ConsoleLogger);
    Log(WINTUN_LOG_INFO, L"Wintun library loaded");

    DWORD LastError;
    HaveQuit = FALSE;
    CloseAppQuit = FALSE;
    QuitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!QuitEvent) {
        LastError = LogError(L"Failed to create event", GetLastError());
        goto cleanupWintun;
    }

    GUID InterfaceGuid;
    UuidCreate(&InterfaceGuid);
    WINTUN_ADAPTER_HANDLE Adapter = WintunCreateAdapter(peer->name, peer->desc, &InterfaceGuid);
    if (!Adapter) {
        LastError = GetLastError();
        LogError(L"Failed to create adapter", LastError);
        goto cleanupQuit;
    }

    DWORD Version = WintunGetRunningDriverVersion();
    Log(WINTUN_LOG_INFO, L"Wintun v%u.%u loaded", (Version >> 16) & 0xff, (Version >> 0) & 0xff);

    WINTUN_SESSION_HANDLE Session = WintunStartSession(Adapter, 0x400000);
    if (!Session) {
        LastError = LogLastError(L"Failed to create adapter");
        goto cleanupAdapter;
    }
    peer->Session = Session;
    Log(WINTUN_LOG_INFO, L"Launching threads and mangling packets...");

    init_peer(peer);

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
        if (!Workers[0] || !Workers[1]) {
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

void destroy_peer() {
    Log(WINTUN_LOG_INFO, L"Cleaning up and shutting down...");
    HaveQuit = TRUE;
    SetEvent(QuitEvent);
    CloseAppQuit = TRUE;
    return TRUE;
}