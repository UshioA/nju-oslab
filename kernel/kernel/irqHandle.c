#include "common.h"
#include "common/list.h"
#include "common/off.h"
#include "common/pool.h"
#include "device.h"
#include "semaphore.h"
#include "socket.h"
#include "x86.h"
#include "x86/memory.h"
#include <stdint.h>

#define SYS_WRITE 0
#define SYS_FORK 1
#define SYS_EXEC 2
#define SYS_SLEEP 3
#define SYS_EXIT 4
#define SYS_READ 5
#define SYS_WAIT 6
#define SYS_TRAP 7
#define SYS_TH_START 8
#define SYS_PID 11
#define SYS_SOCKET 12

extern PDE kpdir[NR_PDE] align_to_page;
extern TSS tss;
extern int displayRow;
extern int displayCol;

extern KEY keyBuffer[MAX_KEYBUFFER_SIZE];
extern volatile int bufferHead;
extern volatile int bufferTail;
extern int32_t _color;
unsigned long long __time;

extern ProcessTable pcb[MAX_PCB_NUM];
extern int current; // current process
void KeyboardHandle(struct StackFrame *tf);
void ClockHandle(struct StackFrame *tf);
void sys_write_stdout(struct StackFrame *tf);
void sys_read(struct StackFrame *tf);
void sys_read_stdin(struct StackFrame *tf);
void syscallGetStr(struct StackFrame *tf);

void sys_trap(struct StackFrame *tf);

void GProtectFaultHandle(struct StackFrame *sf);

void timerHandle(struct StackFrame *sf);

void syscall_handle(struct StackFrame *sf);

void sys_write(struct StackFrame *sf);
// void syscallPrint(struct StackFrame *sf);
void sys_fork(struct StackFrame *sf);
void sys_exec(struct StackFrame *sf);
void sys_sleep(struct StackFrame *sf);
void sys_exit(struct StackFrame *sf);
void sys_wait(struct StackFrame *sf);
void sys_th_start(struct StackFrame *sf);

void sys_socket_new(struct StackFrame *sf);
void sys_socket_destroy(struct StackFrame *sf);
void sys_socket_send(struct StackFrame *sf);
void sys_socket_recv(struct StackFrame *sf);

void sys_socket_handler(struct StackFrame *sf);
void sys_socket_handler(struct StackFrame *sf) {
  switch (sf->ecx) {
  case SOCK_NEW:
    sys_socket_new(sf);
    break;
  case SOCK_DESTROY:
    sys_socket_destroy(sf);
    break;
  case SOCK_SEND:
    sys_socket_send(sf);
    break;
  case SOCK_RECV:
    sys_socket_recv(sf);
    break;
  default:
    break;
  }
}

void sys_socket_new(struct StackFrame *sf) {
  int port = get_socket();
  sf->eax = port;
  Log("\e[34msocket_new\e[0m -> got socket %d", port);
  return;
}

void sys_socket_destroy(struct StackFrame *sf) {
  Log("\e[34msocket_destroy\e[0m -> destroy socket %d", sf->edx);
  back_socket(sf->edx);
  return;
}

void sys_socket_send(struct StackFrame *sf) {
  Log("\e[34msocket_send\e[0m -> from %d send to %d",
      ((udp *)sf->edx)->head.src, ((udp *)sf->edx)->head.dst);
  do_socket_send((void *)sf->edx);
}

void sys_socket_recv(struct StackFrame *sf) {
  udp *pkt = do_socket_recv(sf->edx);
  if (!pkt) {
    sf->eax = -1;
    return;
  }
  Log("\e[34msocket_recv\e[0m -> recv from %d", sf->edx);
  sf->eax = pkt->head.src;
  *(udp *)sf->ebx = *pkt;
  return;
}

void sys_sem_handler(struct StackFrame *sf);
void sys_sem_init(struct StackFrame *sf);
void sys_sem_p(struct StackFrame *sf);
void sys_sem_v(struct StackFrame *sf);
void sys_sem_destroy(struct StackFrame *sf);

void sys_pid(struct StackFrame *sf);

void sys_sem_handler(struct StackFrame *sf);
void sys_sem_handler(struct StackFrame *sf) {
  switch (sf->ecx) {
  case SEM_INIT: {
    pushoff();
    sys_sem_init(sf);
    popoff();
    break;
  }
  case SEM_P: {
    pushoff();
    sys_sem_p(sf);
    popoff();
    break;
  }
  case SEM_V: {
    pushoff();
    sys_sem_v(sf);
    popoff();
    break;
  }
  case SEM_DESTROY: {
    sys_sem_destroy(sf);
    break;
  }
  default:
    break;
  }
}

void sys_sem_init(struct StackFrame *sf) {
  semaphore *pt = pick_sem(&sem_available);
  if (!pt) {
    sf->eax = -1;
    return;
  }
  pt->state = 1;
  sf->eax = pt - sem;
  Log("\e[34msem_init\e[0m -> init sem, value %d, id %d", sf->edx, pt - sem);
  pt->value = (int32_t)sf->edx;
  push_sem(pt, &sem_busy);
}

void sys_sem_p(struct StackFrame *sf) {
  int32_t index = sf->edx;
  Log("\e[34msem_p\e[0m -> P %d", index);
  do_sem_wait(index, SYS_SEM);
}

void sys_sem_v(struct StackFrame *sf) {
  int32_t index = sf->edx;
  Log("\e[34msem_v\e[0m -> V %d", index);
  do_sem_signal(index, SYS_SEM);
}

void sys_sem_destroy(struct StackFrame *sf) {
  pushoff();
  int32_t index = sf->edx;
  sem[index].state = 0;
  sem[index].value = 0;
  list_del_init(&sem[index].list);
  list_add(&sem[index].list, &sem_available.list);
  Log("\e[34msem_destroy\e[0m -> destroyed %d", index);
  popoff();
}

void irqHandle(struct StackFrame *sf) { // pointer sf = esp
  /* Reassign segment register */
  asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));
  /* Save esp to stackTop */
  // uint32_t tmp = pcb[current].stackTop;
  // pcb[current].stackTop = (uint32_t)sf;
  switch (sf->irq) {
  case -1:
    break;
  case 0xd:
    GProtectFaultHandle(sf);
    break;
  case 0x20:
    timerHandle(sf);
    break;
  case 0x80:
    syscall_handle(sf);
    break;
  case 0x21:
    KeyboardHandle(sf);
    break;
  default:
    assert(0);
  }
  // pcb[current].stackTop = tmp;
}

void GProtectFaultHandle(struct StackFrame *sf) {
  Assert(0, "%s", "你越界了傻逼");
  return;
}

static void schedule(struct StackFrame *sf) {
  // now just believe it can schedule well 🙏
  proc *pt = head(busy);
  Assert(pt, "IDLE进程被你吃了傻逼");
  if (pcb[pt->data].state != RUNNING || pt->data == 0) {
    if (pt->next != busy->tail) {
      proc *beg = pt;
      while (pt->data == 0) {
        insert_tail(busy, pick_head(busy));
        pt = head(busy);
        if (pt == beg)
          break;
      }
    }
  }
  pcb[current].stackTop = (uint32_t)sf;
  pcb[pt->data].state = RUNNING;
  tss.esp0 = (uint32_t)(&(pcb[pt->data].stackTop));
  write_cr3(pcb[pt->data].pde);
  popoff();
  current = pt->data;
  // Log("switch to %d", current);
  asm volatile(
      "movl %0, %%esp" ::"m"(pcb[pt->data].stackTop)); // switch kernel stack
  asm volatile("popl %gs");
  asm volatile("popl %fs");
  asm volatile("popl %es");
  asm volatile("popl %ds");
  asm volatile("popal");
  asm volatile("addl $8, %esp");
  asm volatile("iret");
}

void timerHandle(struct StackFrame *sf) {
  // TODO 完成进程调度，建议使用时间片轮转，按顺序调度
  pushoff();
  ++__time;
  proc *iter = head(slp);
  pushoff();
  while (iter && iter != slp->tail) {
    if (pcb[iter->data].sleepTime > 0) {
      --(pcb[iter->data].sleepTime);
      if (!pcb[iter->data].sleepTime) {
        proc *victim = iter;
        iter = iter->last;
        awake(victim);
      }
    }
    iter = iter->next;
  }
  popoff();
  if (pcb[current].state == RUNNING) {
    if (pcb[current].timeCount)
      --pcb[current].timeCount;
    if (!pcb[current].timeCount) {
      pcb[current].state = RUNNABLE;
      insert_tail(busy, pick_head(busy));
      pcb[current].timeCount = MAX_TIME_COUNT;
    }
  }
  schedule(sf);
}

void KeyboardHandle(struct StackFrame *tf) {
  uint32_t code = getKeyCode();
  if (code < 0x81) {
    if (code == 0xe) {
      keyBuffer[bufferTail++] = (KEY){code, BACKSPACE};
    } else if (code == 0x1c) {
      keyBuffer[bufferTail++] = (KEY){code, NEWLINE};
    } else {
      char character = getChar(code);
      if (!character)
        return;
      keyBuffer[bufferTail++] = (KEY){character, NORMAL};
    }
    if (dev[stdin].state)
      do_sem_signal(stdin, SYS_DEV);
  }
  bufferTail %= MAX_KEYBUFFER_SIZE;
}

void syscall_handle(struct StackFrame *sf) {
  switch (sf->eax) { // syscall number
  case 0:
    pushoff();
    sys_write(sf);
    popoff();
    break; // for SYS_WRITE
  case 1:
    sys_fork(sf);
    break;
  case 2:
    sys_exec(sf);
    break;
  case 3:
    sys_sleep(sf);
    break;
  case 4:
    sys_exit(sf);
    break;
  case 5:
    sys_read(sf);
    break;
  case 6:
    sys_wait(sf);
    break;
  case 7:
    pushoff();
    sys_trap(sf);
    popoff();
    break;
  case 8:
    sys_th_start(sf);
    break;
  case SYS_SEM:
    sys_sem_handler(sf);
    break;
  case SYS_PID:
    sys_pid(sf);
    break;
  case SYS_SOCKET:
    sys_socket_handler(sf);
    break;
  default:
    break;
  }
}

void sys_pid(struct StackFrame *sf) {
  sf->eax = current;
  return;
}

void sys_th_start(struct StackFrame *sf) {
  int32_t pid = sf->ecx;
  if (pcb[pid].state == RUNNABLE) {
    sf->eax = -1;
    return;
  }
  Log("starting thread, pid %d", pid);
  if (pcb[pid].state != DEAD || pcb[pid].state != ZOMBIE) {
    pcb[pid].state = RUNNABLE;
    pcb[pid].sleepTime = 0;
    free_proc(slp, pcb_pts + pid);
    insert_tail(busy, pcb_pts + pid);
  }
}

void sys_wait(struct StackFrame *sf) {
  uint32_t wait_flag = sf->edx;
  for (;;) {
    char has_kid = 0;
    if (wait_flag == -1) {
      pushoff();
      for (int32_t i = 0; i < MAX_PCB_NUM; ++i) {
        if (pcb[i].parent != current)
          continue;
        has_kid = 1;
        if (pcb[i].state == ZOMBIE) {
          sf->eax = i;
          if (sf->ecx) {
            *(int32_t *)(sf->ecx) = pcb[i].regs.eax;
          }

          Log("\e[34mwait\e[0m -> %d find zombie child %d", current, i);
          pushoff();
          kill_zombie(pcb_pts + i);
          popoff();
          pcb[i].parent = -1;
          popoff();
          return;
        }
      }

      if (!has_kid) {
        sf->eax = -1;
        popoff();
        return;
      }
      Log("\e[34mwait\e[0m -> %d find no zombie, sleep", current);
      set_sleep(pcb_pts + current, -1); // wait for its child to wake it up
      popoff();
      asm volatile("int $0x20");
    } else {
      if (pcb[wait_flag].parent != current || pcb[wait_flag].state == BABY) {
        sf->eax = -1; // wait 错了
        return;
      } else {
        if (pcb[wait_flag].state == ZOMBIE) {
          if (sf->ecx) {
            *(int32_t *)(sf->ecx) = pcb[wait_flag].regs.eax;
          }
          pcb[wait_flag].parent = -1;
          Log("\e[34mwait\e[0m -> %d find zombie child thread %d", current,
              wait_flag);
          pcb[wait_flag].state = DEAD;
          pushoff();
          deallocuvm((void *)pcb[wait_flag].pde,
                     PGROUNDUP(pcb[wait_flag].ustacktop),
                     pcb[wait_flag].ustacktop - 2 * PGSIZE);
          free_proc(zombie, (pcb_pts + wait_flag));
          insert_tail(avai, (pcb_pts + wait_flag));
          popoff();
          return;
        } else {
          Log("\e[34mwait\e[0m -> %d find no zombie, sleep", current);
          pushoff();
          set_sleep(pcb_pts + current, -1);
          popoff();
          asm volatile("int $0x20");
        }
      }
    }
  }
}

void sys_trap(struct StackFrame *sf) { shutdown(); }

void sys_write(struct StackFrame *sf) {
  switch (sf->ecx) { // file descriptor
  case stdout:
    if (dev[stdout].state) {
      pushoff();
      sys_write_stdout(sf);
      popoff();
    }
    break; // for STD_OUT
  default:
    break;
  }
}

static void update_row(int32_t num) {
  displayRow += num * FONT_H;
  if (displayRow == SCR_HEIGHT) {
    scrollScreen();
    displayRow = SCR_HEIGHT - FONT_H;
    displayCol = 0;
  }
}

static void update_col() {
  if (displayCol == SCR_WIDTH) {
    displayCol = 0;
    update_row(1);
  } else if (displayCol < 0) {
    displayCol = SCR_WIDTH - (SCR_WIDTH % FONT_W);
    if (displayRow > 0)
      update_row(-1);
  }
}

static void put(char c) {
  uint8_t data = c;
  if (c == '\r') {
    displayCol = 0;
    update_col();
  } else if (c == '\b') {
    displayCol -= FONT_W;
    update_col();
  } else if (c == '\n') {
    displayCol = 0;
    update_row(1);
  } else if (c == '\t') {
    put(' ');
  } else {
    if (displayCol + FONT_W > SCR_WIDTH) {
      displayCol = SCR_WIDTH;
      update_col();
    }
    draw_character(data, displayRow, displayCol, _color);
    displayCol += FONT_W;
    update_col();
  }
#ifdef TEXT_VGA
  updateCursor(displayRow, displayCol);
#endif
#ifdef COMMAND_LINE
  putChar(data);
#endif
}

void sys_write_stdout(struct StackFrame *tf) {
  char *str = (char *)tf->edx;
  int size = tf->ebx;
  int i = 0;
  char character = 0;
  for (i = 0; i < size; i++) {
    character = str[i];
    put(character);
  }
}

void sys_fork(struct StackFrame *sf) {
  pushoff();
  proc *a = get_proc();
  if (!a) {
    sf->eax = -1;
    popoff();
    return;
  }
  sf->eax = pcb[current].regs.eax = a->data;
  uint32_t fork_flag = sf->ecx;
  Log("\e[34m%sfork\e[0m -> %d find available pid : %d",
      (fork_flag ? "" : "[new_thread]"), current, a->data);
  popoff();
  if (fork_flag) {
    pushoff();
    PDE *new_pde = copyuvm((void *)pcb[current].pde,
                           (uint32_t)va_to_pa(pcb[current].ustacktop));
    memcpy((void *)&(pcb[a->data]), (void *)&pcb[current],
           sizeof(ProcessTable));
    activate(a);
    pcb[a->data].pde = (uint32_t)new_pde;
    pcb[a->data].parent = current;
    pcb[a->data].regs.eax = 0;
    popoff();
  } else {
    pushoff();
    uint32_t func = sf->edx;
    int argc = (int32_t)sf->ebx;
    void *argv = (void *)sf->esi;
    ++pcb[current].threads;
    pcb[a->data].parent = current;
    for (int i = 0; i < MAX_PCB_NUM; ++i) {
      if (pcb[i].parent == current) {
        pcb[i].threads = pcb[current].threads;
      }
    }
    activate((a));
    pcb[a->data].state = BABY;
    pcb[a->data].sleepTime = -1;
    free_proc(busy, (a));
    insert_tail(slp, a);
    pcb[a->data].pde = pcb[current].pde;
    assert(alloc_umem(pcb[a->data].pde, PGROUNDUP(pcb[current].ustacktop),
                      pcb[current].ustacktop + 2 * PGSIZE) ==
           pcb[current].ustacktop + 2 * PGSIZE);
    memset((void *)PGROUNDUP(pcb[current].ustacktop), 0, 2 * PGSIZE);
    pcb[a->data].ustacktop = PGROUNDUP(pcb[current].ustacktop) + 2 * PGSIZE;
    pcb[a->data].regs.esp = pcb[a->data].regs.ebp = pcb[a->data].ustacktop;
    pcb[a->data].regs.esp -= (argc + 1) * sizeof(void *);
    uint32_t *ustack = (void *)pcb[a->data].regs.esp;
    ustack[0] = 0xffffffff; // fake return
    for (int i = 0; i < argc; ++i) {
      ustack[i + 1] = ((uint32_t *)argv)[i];
    }
    pcb[a->data].regs.eip = func;
    popoff();
  }
  asm volatile("int $0x20");
}

void sys_exec(struct StackFrame *sf) {
  if (pcb[current].parent != -1 &&
      pcb[current].pde == pcb[pcb[current].parent].pde) {
    Say("\e[34mexec\e[0m -> 坏孩子😡");
    sf->eax = -1;
    return;
  }

  char *name = (void *)sf->ecx;
  int argc = (int32_t)sf->edx;
  char **argv = (void *)sf->ebx;

  int i = find_file(name);
  uint32_t sec_start, sec_num;
  if (i == -1) {
    Log("\e[34mexec\e[0m -> executable not found, stop.");
    sf->eax = -1;
    return;
  }
  sec_start = file_info_table[i].sec_start;
  sec_num = file_info_table[i].sec_len;
  uint32_t space_end = (uint32_t)pa_to_va(sec_num * 512 + 2 * PAGE_SIZE);
  Log("\e[34mexec\e[0m -> found executable, start at %d, length %d", sec_start,
      sec_num);
  if (sec_num > pcb[current].sz) {
    alloc_umem(
        pcb[current].pde,
        (uint32_t)pa_to_va(PGROUNDUP((pcb[current].sz) * 512 + 2 * PGSIZE)),
        space_end);
  }
  uint32_t stack = pcb[current].ustacktop = space_end;
  stack -= (3 + argc) * sizeof(void *);
  uint32_t *ustack = (void *)stack;
  ustack[0] = 0xffffffff;
  ustack[1] = argc;
  ustack[2] = (uint32_t) & (ustack[3]);
  char *start = pa_to_va(PGROUNDUP(sec_num * 512));
  memset(start, 0, PGSIZE);
  for (int i = 0; i < argc; ++i) {
    memcpy(start, argv[i], strlen(argv[i]));
    ustack[3 + i] = (uint32_t)start;
    start = start + strlen(argv[i]) + 1;
  }
  uint32_t eip = loadelf(sec_start, sec_num, 0);
  pcb[current].regs.eip = eip;
  pcb[current].start = sec_start;
  pcb[current].sz = sec_num;
  pcb[current].regs.esp = (uint32_t)ustack;
  Log("\e[34mexec\e[0m -> exec on pid %d", current);
  *sf = pcb[current].regs;
  pushoff();
  schedule(sf);
}

void sys_sleep(struct StackFrame *sf) {
  pushoff();
  uint32_t time = sf->ecx;
  set_sleep(pcb_pts + current, time);
  Log("\e[34msleep\e[0m -> set pid %d sleep %d", current, time);
  popoff();
  asm volatile("int $0x20");
}

void sys_exit(struct StackFrame *sf) {
  int32_t ret = sf->ecx;
  pushoff();
  kill((pcb_pts + current));
  popoff();
  pcb[current].regs.eax = sf->eax = ret;
  Assert((pcb_pts + current)->data == current,
         "傻逼你杀错了, 应该是 %d, 你杀了 %d", current,
         (pcb_pts + current)->data);
  if (pcb[current].parent != -1 &&
      pcb[(pcb_pts + (pcb[current].parent))->data].state == BLOCKED) {
    pushoff();
    awake(pcb_pts + pcb[current].parent);
    popoff();
  } else if (pcb[current].parent == -1) { //没爹妈就直接🦈了
    pushoff();
    write_cr3((uint32_t)kpdir);
    kill_zombie(pcb_pts + current);
    popoff();
  }
  Log("\e[34mexit\e[0m -> pid %d exited with code %d",
      (pcb_pts + current)->data, ret);
  asm volatile("int $0x20");
}

void sys_read(struct StackFrame *tf) {
  switch (tf->ecx) { // file descriptor
  case stdin:
    if (dev[stdin].state)
      sys_read_stdin(tf);
    break; // for STD_IN
  case 2:
    syscallGetStr(tf);
    break; // for STD_STR
  default:
    break;
  }
}

void sys_read_stdin(struct StackFrame *tf) {
  if (dev[stdin].value < 0 || !dev[stdin].state) {
    tf->eax = -1;
    return;
  }
  char c = 0;
  do_sem_wait(stdin, SYS_DEV);
  keytype t = keyBuffer[bufferHead].type;
  switch (t) {
  case NORMAL: {
    c = keyBuffer[bufferHead++].code;
    break;
  }
  case BACKSPACE: {
    c = 0x7f;
    ++bufferHead;
    break;
  }
  case NEWLINE: {
    c = '\n';
    ++bufferHead;
    break;
  }
  default:
    break;
  }
  bufferHead %= MAX_KEYBUFFER_SIZE;
  tf->eax = c;
  return;
}

static KEY get_one() {
  volatile char sb = (bufferHead == bufferTail);
  sti();
  while ((sb = (bufferHead == bufferTail)))
    hlt();
  KEY k = keyBuffer[bufferHead++];
  bufferHead %= MAX_KEYBUFFER_SIZE;
  return k;
}

static void copy_buf(const KEY *buf, const char *str, uint32_t ds) {
  int i = 0;
  int selector = ds;
  asm volatile("movw %0, %%es" ::"m"(selector));
  int k = 0;
  for (int p = 0; buf[p].type != NR_keytype; ++p) {
    asm volatile("movl %0, %%es:(%1)" ::"r"(buf[p].code), "r"(str + k));
    ++k;
  }
  asm volatile("movb $0x00, %%es:(%0)" ::"r"(str + i));
}

void syscallGetStr(struct StackFrame *tf) {
  char *str = (char *)(tf->edx); // str pointer
  int size = (int)(tf->ebx);     // str size
  int i = 0;
  KEY get = {0, NR_keytype};
  KEY buf[size + 1];
  while (i < size) {
    get = get_one();
    if (get.type == NEWLINE)
      break;
    buf[i++] = get;
  }
  buf[i] = (KEY){0, NR_keytype};
  copy_buf(buf, str, tf->ds);
  return;
}