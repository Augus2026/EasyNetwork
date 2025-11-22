#ifndef WINTUN_PEER_H
#define WINTUN_PEER_H

#include "wintun.h"

#define WOLFSSL_USER_SETTINGS
#include "wolfssl/ssl.h"

#include "wintun_init.h"
#include "tls_client.h"
#include "hdr.h"
#include "itf.h"

void init_peer(struct peer_info_t* peer);
int build_peer(struct peer_info_t* peer);
void destroy_peer();

#endif // WINTUN_PEER_H