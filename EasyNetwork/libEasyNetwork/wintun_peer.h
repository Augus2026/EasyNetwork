#ifndef WINTUN_PEER_H
#define WINTUN_PEER_H

#include "wintun_init.h"
#include "tls_client.h"
#include "hdr.h"
#include "itf.h"

#include "wintun.h"

#define WOLFSSL_USER_SETTINGS
#include "wolfssl/ssl.h"

// 初始化peer
int build_peer(struct peer_info_t* peer);

// 销毁peer
void destroy_peer();

#endif // WINTUN_PEER_H