#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#pragma pack(1)
typedef enum message_type_ {
	REGISTER_PEER = 1,
	TRANSPORT_DATA,
	PING,
	PONG,
} message_type_t;

typedef struct message_fixed_head_ {
	unsigned short magic;
	unsigned char msgtype;
	union {
		struct {
			struct in_addr src_addr;
			struct in_addr dst_addr;
		};
		struct in_addr register_addr;
		struct {
			unsigned int code;
			unsigned int clock;
		};
	};
	unsigned short size;
} message_fixed_head_t;

typedef struct message_head_ {
	message_fixed_head_t head;
	char data[0];
} message_head_t;
#pragma pack()

#endif // MESSAGE_H