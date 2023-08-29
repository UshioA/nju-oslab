#ifndef __UDP__
#define __UDP__

#include <stdint.h>

typedef struct {
  uint16_t src;
  uint16_t dst;
  uint16_t cksum;
} udp_head;

typedef struct {
  udp_head head;
  uint8_t packet[512 - sizeof(udp_head)];
} udp;
#endif