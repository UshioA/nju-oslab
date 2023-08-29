#ifndef __PCB_H__
#define __PCB_H__
#include "common.h"
#include "common/pool.h"
#include "x86/memory.h"
#include <stdint.h>

struct ProcessTable {
  uint32_t stack[MAX_STACK_SIZE];
  struct StackFrame regs;
  uint32_t stackTop;
  uint32_t threads;
  int state;
  int timeCount;
  int sleepTime;
  uint32_t pid;
  int parent;
  uint32_t pde;
  uint8_t intena;
  uint32_t ncli;
  uint32_t start;
  uint32_t sz;
  uint32_t ustacktop;
  list_entry link;
  char name[32];
};
typedef struct ProcessTable ProcessTable;

extern proc pcb_pts[MAX_PCB_NUM]; // pointers

extern pool _busy;
extern pool _avai;
extern pool _sleep;
extern pool _zombie;

extern pool *busy;
extern pool *avai;
extern pool *slp;
extern pool *zombie;

extern proc _bh, _bt, _ah, _at, _sh, _st, _zh, _zt;

// proc *get_proc();
#define get_proc() pick_head(avai)

#define kill_zombie(p)                                                         \
  do {                                                                         \
    pcb[(p)->data].state = DEAD;                                               \
    memset((void *)pcb[(p)->data].pde, 0, PHY_MEM / PT_SIZE);                  \
    freevm((void *)pcb[(p)->data].pde, (uint32_t)pcb[(p)->data].ustacktop,     \
           (uint32_t)pa_to_va(0));                                             \
    free_proc(zombie, (p));                                                    \
    insert_tail(avai, (p));                                                    \
  } while (0)

#define make_zombie(p)                                                         \
  do {                                                                         \
    free_proc(busy, (p));                                                      \
    insert_tail(zombie, (p));                                                  \
  } while (0)

#define activate(p)                                                            \
  do {                                                                         \
    pcb[p->data].regs.cs = USEL(SEG_UCODE);                                    \
    pcb[p->data].regs.ds = USEL(SEG_UDATA);                                    \
    pcb[p->data].regs.es = USEL(SEG_UDATA);                                    \
    pcb[p->data].regs.fs = USEL(SEG_UDATA);                                    \
    pcb[p->data].regs.gs = USEL(SEG_UDATA);                                    \
    pcb[p->data].regs.ss = USEL(SEG_UDATA);                                    \
    pcb[p->data].stackTop = (uint32_t) & (pcb[p->data].regs);                  \
    pcb[p->data].state = RUNNABLE;                                             \
    pcb[p->data].timeCount = MAX_TIME_COUNT;                                   \
    pcb[p->data].sleepTime = 0;                                                \
    pcb[p->data].pid = p->data;                                                \
    insert_tail(busy, (p));                                                    \
  } while (0)

#define kill(p)                                                                \
  do {                                                                         \
    pcb[(p)->data].state = ZOMBIE;                                             \
    make_zombie((p));                                                          \
  } while (0)

#define set_sleep(p, sec)                                                      \
  do {                                                                         \
    pcb[(p)->data].state = BLOCKED;                                            \
    pcb[(p)->data].sleepTime = sec;                                            \
    free_proc(busy, (p));                                                      \
    push_proc(slp, (p));                                                       \
  } while (0)

#define awake(p)                                                               \
  do {                                                                         \
    pcb[(p)->data].state = RUNNABLE;                                           \
    pcb[(p)->data].sleepTime = 0;                                              \
    free_proc(slp, (p));                                                       \
    push_proc(busy, (p));                                                      \
  } while (0)

enum pcb_state { RUNNABLE, RUNNING, BLOCKED, BABY, DEAD, ZOMBIE, NR_PCB_STATE };

#endif