#ifndef EASY_NETWORK_FFI_H
#define EASY_NETWORK_FFI_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

EXPORT void set_server(
    const char* reply_address,
    int reply_port); 

EXPORT void join_network(
    const char* ifname,
    const char* ifdesc,
    const char* ip,
    const char* netmask,
    const char* mtu,
    const char* domain,
    const char* nameServer,
    const char* searchList);

EXPORT void leave_network();

EXPORT void add_route(
    const char* destination,
    const char* netmask,
    const char* gateway,
    const char* metric);

EXPORT void clean_route();

#ifdef __cplusplus
}
#endif

#endif // EASY_NETWORK_FFI_H
