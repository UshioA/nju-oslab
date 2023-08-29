#ifndef __POOL__
#define __POOL__
#include "types.h"
#include <stdint.h>
typedef struct node {
  uint32_t data;
  struct node *next;
  struct node *last;
} node;

typedef node proc;

typedef struct pool {
  node *head;
  node *tail;
  int size;
} pool;

#define head(l) (l->size) ? ((l)->head->next) : (NULL)
#define end(l) (l->size) ? ((l)->tail->last) : (NULL)

#define init_pool(l, a, b)                                                     \
  do {                                                                         \
    (l)->head = a;                                                             \
    (l)->tail = b;                                                             \
    (l)->size = 0;                                                             \
    (l)->head->next = (l)->tail;                                               \
    (l)->tail->last = (l)->head;                                               \
    (l)->head->last = (l)->tail->next = NULL;                                  \
  } while (0)

inline void push_proc(pool *l, proc *p) {
  ++(l->size);
  p->next = l->tail;
  p->last = l->tail->last;
  l->tail->last->next = p;
  l->tail->last = p;
}

inline void insert_head(pool *l, proc *p) {
  ++(l->size);
  p->last = l->head;
  p->next = l->head->next;
  p->next->last = p;
  l->head->next = p;
}

inline void insert_tail(pool *l, proc *p) {
  ++(l->size);
  p->next = l->tail;
  p->last = l->tail->last;
  p->last->next = p;
  l->tail->last = p;
}

inline proc *pick_head(pool *l) {
  proc *hd = head(l);
  if (hd) {
    hd->last->next = hd->next;
    hd->next->last = hd->last;
    hd->last = hd->next = NULL;
  } else
    return hd;
  --(l->size);
  return hd;
}

inline void free_proc(pool *l, proc *p) {
  //* here assume free is always valid, please ensure this outside=)
  --(l->size);
  p->last->next = p->next;
  p->next->last = p->last;
  p->last = p->next = NULL;
}

#define pop_proc(l)                                                            \
  do {                                                                         \
    free_proc((l), end((l)));                                                  \
  } while (0)

#endif