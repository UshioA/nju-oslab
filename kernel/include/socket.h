#ifndef __SOCKET__
#define __SOCKET__
#include "common.h"
#include <stdint.h>

typedef struct {
  uint16_t port;
  udp_queue buf;
  uint8_t in_use;
} socket;

#define MAX_SOCKET 64

extern socket socks[MAX_SOCKET];

void init_socket();

int get_socket();

void back_socket(int);

void do_socket_send(udp *msg);

udp* do_socket_recv(int port);

#define SOCK_NEW 0
#define SOCK_DESTROY 1
#define SOCK_SEND 2
#define SOCK_RECV 3

#endif