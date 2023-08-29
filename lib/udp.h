#ifndef __UDP__
#define __UDP__
#include "types.h"
#define UDP_LEN 512
#define PAYLOAD_LEN UDP_LEN - sizeof(udp_head)

typedef struct {
  uint16_t src;
  uint16_t dst;
  uint16_t cksum;
} udp_head;

typedef struct {
  udp_head head;
  uint8_t packet[PAYLOAD_LEN];
} udp;
#endif