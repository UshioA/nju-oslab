#ifndef __SEM__
#define __SEM__
#include "common.h"
#include "device.h"
#include "x86.h"
#include <stdint.h>

#define MAX_SEMAPHORE 128
#define MAX_DEVICE 128

#define SYS_SEM 9
#define SYS_DEV 10

#define SEM_INIT 0
#define SEM_P 1
#define SEM_V 2
#define SEM_DESTROY 3

typedef struct semaphore {
  int value;
  int state;
  list_entry list;
  list_entry process;
} semaphore;

typedef semaphore device;

inline void sem_init(semaphore *h) {
  h->value = 0;
  h->state = 0;
  list_init(&h->list);
  list_init(&h->process);
}

inline semaphore *pick_sem(semaphore *head) {
  if (list_empty(&head->list))
    return NULL;
  semaphore *p = le2(semaphore, head->list.next, list);
  list_del_init(head->list.next);
  --head->value;
  return p;
}

//* user himself make sure `entry' and `to' is safe and valid :)
inline void push_sem(semaphore *entry, semaphore *to) {
  list_add_tail(&entry->list, &to->list);
  ++to->value;
}

#define dev_init sem_init
#define pick_dev pick_sem
#define push_dev push_sem

extern semaphore sem_busy, sem_available;
extern device dev_busy, dev_available;

extern semaphore sem[MAX_SEMAPHORE];
extern device dev[MAX_DEVICE];

void init_sem();
void init_dev();
void do_sem_wait(int32_t, int32_t);
void do_sem_signal(int32_t, int32_t);
#endif