#include "common.h"
#include "x86.h"
#include "x86/cpu.h"
#include <stdint.h>
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;
void pushoff() {
  uint32_t IF = (read_eflags() & FL_IF) == FL_IF;
  cli();
  ++pcb[current].ncli;
  if (pcb[current].ncli == 1) {
    pcb[current].intena = IF;
  }
}

void popoff() {
  --pcb[current].ncli;
  Assert((int)pcb[current].ncli >= 0, "傻逼你pop多了");
  uint32_t eflags = read_eflags();
  uint32_t IF = (eflags & FL_IF) == FL_IF;
  Assert(!IF, "怎么会是呢");
  if (!pcb[current].ncli && pcb[current].intena) {
    sti();
  }
}