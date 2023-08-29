#include "socket.h"
#include "lib.h"
#include "types.h"
#include "znrlib.h"

int socket_recv(sock_t port, udp *recv) { return __socket_recv(port, recv); }

void socket_send(uint8_t *data, int32_t length, sock_t from, sock_t to) {
  udp buf;
  buf.head.src = from, buf.head.dst = to;
  while (length > 0) {
    memcpy(&buf.packet, data, PAYLOAD_LEN < length ? PAYLOAD_LEN : length);
    data += PAYLOAD_LEN;
    length -= PAYLOAD_LEN;
    __socket_send(&buf);
  }
}