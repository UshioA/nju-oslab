#include "socket.h"
#include "common.h"
#include "common/queue.h"
#include <stdint.h>

socket socks[MAX_SOCKET];

void init_socket() {
  for (int i = 0; i < MAX_SOCKET; ++i) {
    socks[i].port = i;
    socks[i].in_use = 0;
    init_udp_queue(&(socks[i].buf));
  }
}

int get_socket() {
  for (int i = 0; i < MAX_SOCKET; ++i) {
    if (!socks[i].in_use) {
      socks[i].in_use = 1;
      return i;
    }
  }
  return -1;
}

void back_socket(int index) {
  socks[index].in_use = 0;
  init_udp_queue(&socks[index].buf);
}

void do_socket_send(udp *msg) {
  uint16_t from_port = msg->head.src;
  uint16_t to_port = msg->head.dst;
  if (!socks[from_port].in_use && !socks[to_port].in_use)
    return;
  push_queue(&socks[to_port].buf, msg);
}

udp *do_socket_recv(int port) {
  return pop_queue(&socks[port].buf);
}