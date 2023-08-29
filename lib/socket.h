#ifndef __USOCK__
#define __USOCK__
#include "lib.h"
#include "types.h"

void socket_send(uint8_t *data, int32_t length, sock_t from, sock_t to);

int socket_recv(sock_t port, udp *recv);

#endif