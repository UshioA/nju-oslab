#include "../include/semaphore.h"
#include "common/list.h"
#include "common/off.h"
#include "x86/pcb.h"
#include <stdint.h>

extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

semaphore sem_busy;
semaphore sem_available;

device dev_busy;
device dev_available;

semaphore sem[MAX_SEMAPHORE];
device dev[MAX_DEVICE];

void init_sem() {
  sem_init(&sem_busy);
  sem_init(&sem_available);
  for (int i = 0; i < MAX_SEMAPHORE; ++i) {
    sem_init(sem + i);
    push_sem(sem + i, &sem_available);
  }
}

void init_dev() {
  dev_init(&dev_busy);
  dev_init(&dev_available);
  for (int i = 0; i < MAX_DEVICE; ++i) {
    dev_init(dev + i);
    dev[i].state = 1;
    push_dev(dev + i, &dev_busy);
  }
}

void do_sem_wait(int32_t i, int32_t class) {
  pushoff();
  semaphore *s = (class == SYS_SEM ? sem : dev) + i;
  if (--s->value < 0) {
    list_add_tail(&(pcb[current].link), &(s->process));
    if (class == SYS_SEM)
      Log("\e[34mdo_sem_wait\e[0m -> blocked proc %d", current);
    set_sleep(pcb_pts + current, -1);
  }
  popoff();
  asm volatile("int $0x20");
}

void do_sem_signal(int32_t i, int32_t class) {
  pushoff();
  semaphore *s = (class == SYS_SEM ? sem : dev) + i;
  if (++s->value <= 0) {
    list_entry *pt = list_head(&s->process);
    if (!pt)
      goto ret;
    list_del_init(pt);
    awake(pcb_pts + (le2(ProcessTable, pt, link) - pcb));
  }
ret:
  popoff();
}