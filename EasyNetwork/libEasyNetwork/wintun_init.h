#ifndef WINTUN_INIT_H
#define WINTUN_INIT_H

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

WINTUN_CREATE_ADAPTER_FUNC *WintunCreateAdapter;
WINTUN_CLOSE_ADAPTER_FUNC *WintunCloseAdapter;
WINTUN_OPEN_ADAPTER_FUNC *WintunOpenAdapter;
WINTUN_GET_ADAPTER_LUID_FUNC *WintunGetAdapterLUID;
WINTUN_GET_RUNNING_DRIVER_VERSION_FUNC *WintunGetRunningDriverVersion;
WINTUN_DELETE_DRIVER_FUNC *WintunDeleteDriver;
WINTUN_SET_LOGGER_FUNC *WintunSetLogger;
WINTUN_START_SESSION_FUNC *WintunStartSession;
WINTUN_END_SESSION_FUNC *WintunEndSession;
WINTUN_GET_READ_WAIT_EVENT_FUNC *WintunGetReadWaitEvent;
WINTUN_RECEIVE_PACKET_FUNC *WintunReceivePacket;
WINTUN_RELEASE_RECEIVE_PACKET_FUNC *WintunReleaseReceivePacket;
WINTUN_ALLOCATE_SEND_PACKET_FUNC *WintunAllocateSendPacket;
WINTUN_SEND_PACKET_FUNC *WintunSendPacket;

HMODULE
InitializeWintun(void);

void CALLBACK
ConsoleLogger(_In_ WINTUN_LOGGER_LEVEL Level, _In_ DWORD64 Timestamp, _In_z_ const WCHAR *LogLine);

DWORD64
Now(VOID);

DWORD
LogError(_In_z_ const WCHAR *Prefix, _In_ DWORD Error);

DWORD
LogLastError(_In_z_ const WCHAR *Prefix);

void
Log(_In_ WINTUN_LOGGER_LEVEL Level, _In_z_ const WCHAR *Format, ...);

#endif // WINTUN_INIT_H