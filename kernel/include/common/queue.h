#ifndef __QUEUE__
#define __QUEUE__
#include "common/assert.h"
#include "common/debug.h"
#include "udp.h"
#include <stdint.h>
#define MAX_UDP_BUFLEN 16
typedef struct {
  uint32_t begin;
  uint32_t end;
  uint32_t len;
  uint32_t size;
  udp data[MAX_UDP_BUFLEN];
} udp_queue;

inline void init_udp_queue(udp_queue *q) {
  q->begin = q->end = q->len = 0;
  q->size = MAX_UDP_BUFLEN;
}

inline uint32_t len_queue(udp_queue *q) { return q->len; }

inline int queue_empty(udp_queue *q) { return q->begin == q->end; }

inline int queue_full(udp_queue *q) {
  return (q->end + 1) % q->size == q->begin;
}

inline void queue_increase_end(udp_queue *q) {
  q->end = (q->end + 1) % q->size;
}

inline void queue_increase_begin(udp_queue *q) {
  q->begin = (q->begin + 1) % q->size;
}

inline int push_queue(udp_queue *q, udp *data) {
  if (queue_full(q)) //* drop packet
    return -1;
  q->data[q->end] = *data;
  ++q->len;
  queue_increase_end(q);
  return 0;
}

inline udp *pop_queue(udp_queue *q) {
  if (queue_empty(q)) {
    return (void *)0;
  }
  udp *pkt = &(q->data[q->begin]);
  queue_increase_begin(q);
  --q->len;
  return pkt;
}

#endif