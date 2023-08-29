#include "common.h"
#include "common/assert.h"
#include "common/off.h"
#include "common/pool.h"
#include "device.h"
#include "device/serial.h"
#include "device/vedio.h"
#include "x86.h"
#include "x86/cpu.h"
#include "x86/fs.h"
#include "x86/io.h"
#include "x86/memory.h"
#include "x86/page.h"
#include "x86/pcb.h"
#include <stdint.h>

file_info file_info_table[] = {
#include "common/file_info.h"
};

SegDesc
    gdt[NR_SEGMENTS]; // the new GDT, NR_SEGMENTS=10, defined in x86/memory.h
TSS tss;
ProcessTable pcb[MAX_PCB_NUM]; // pcb

uint8_t elf[0x80000];

int current; // current process

PDE kpdir[NR_PDE] align_to_page; // kernel page dirctory
PTE kptable[PHY_MEM / PAGE_SIZE] align_to_page;

PDE *get_kpdir() { return kpdir; }

int32_t find_file(char *target) {
  for (int i = 0; i < NR_FILE; ++i) {
    if (!strcmp(target, file_info_table[i].name)) {
      return i;
    }
  }
  return -1;
}

void init_page(void) {
  CR0 cr0;
  CR3 cr3;
  PDE *pdir = (PDE *)(kpdir);
  PTE *ptable = (PTE *)(kptable);
  uint32_t pdir_idx, ptable_idx, pframe_idx;

  // make all PDE invalid
  memset(pdir, 0, NR_PDE * sizeof(PDE));

  // fill PDE and PTE
  pframe_idx = 0;
  for (pdir_idx = 0; pdir_idx < PHY_MEM / PT_SIZE; ++pdir_idx) {
    pdir[pdir_idx].val = make_pde(ptable);
    // pdir[pdir_idx + KOFFSET / PT_SIZE].val = make_pde(ptable);
    for (ptable_idx = 0; ptable_idx < NR_PTE; ++ptable_idx) {
      (ptable++)->val = make_pte((pframe_idx++) << 12);
    }
  }

  cr3.val = 0;
  cr3.page_directory_base = ((uint32_t)pdir >> 12);
  write_cr3(cr3.val);

  cr0.val = read_cr0();
  cr0.paging = 1;
  write_cr0(cr0.val);
}

void initSeg() { // setup kernel segements
  // gdt[1] and gdt[2]
  gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  gdt[SEG_TSS] = SEG16(STS_T32A, &tss, sizeof(TSS) - 1, DPL_KERN);
  gdt[SEG_TSS].s = 0; // system

  setGdt(gdt,
         sizeof(gdt)); // gdt is set in bootloader, here reset gdt in kernel

  /* initialize TSS */
  tss.ss0 = KSEL(SEG_KDATA);
  asm volatile("ltr %%ax" ::"a"(KSEL(SEG_TSS)));

  /* reassign segment register */
  asm volatile("movw %%ax,%%ds" ::"a"(KSEL(SEG_KDATA)));
  asm volatile("movw %%ax,%%ss" ::"a"(KSEL(SEG_KDATA)));

  lLdt(0);
}

uint32_t loadUMain();

static void idle() {
  sti();
  while (1)
    hlt();
}

void initProc() {
  int i = 0;
  for (i = 0; i < MAX_PCB_NUM; i++) {
    pcb[i].state = DEAD;
    pcb[i].parent = -1;
    list_init(&pcb[i].link);
  }
  // kernel process
  pcb[0].stackTop = (uint32_t) & (pcb[0].stackTop);
  pcb[0].state = RUNNING;
  pcb[0].timeCount = 0;
  pcb[0].sleepTime = 0;
  pcb[0].pid = 0;
  pcb[0].intena = 1;
  pcb[0].ncli = 0;
  pcb[0].pde = (uint32_t)kpdir;
  pcb[0].regs.eip = (uint32_t)idle;
  // user process
  pcb[1].stackTop = (uint32_t) & (pcb[1].regs);
  pcb[1].state = RUNNABLE;
  pcb[1].timeCount = MAX_TIME_COUNT;
  pcb[1].sleepTime = 0;
  pcb[1].pid = 1;
  pcb[1].regs.ss = USEL(4);
  asm volatile("pushfl");
  asm volatile("popl %0" : "=r"(pcb[1].regs.eflags));
  pcb[1].regs.eflags = pcb[1].regs.eflags | 0x200;
  pcb[1].regs.cs = USEL(SEG_UCODE);
  pcb[1].regs.ds = USEL(SEG_UDATA);
  pcb[1].regs.es = USEL(SEG_UDATA);
  pcb[1].regs.fs = USEL(SEG_UDATA);
  pcb[1].regs.gs = USEL(SEG_UDATA);
  pcb[1].intena = 1;
  pcb[1].ncli = 0;

  uint32_t sec_start, sec_num;

  i = find_file("app.elf");
  if (i < 0) {
    Assert(0, "start file not found");
  } else {
    sec_start = file_info_table[i].sec_start;
    sec_num = file_info_table[i].sec_len;
  }

  pcb[1].pde = kalloc();
  pcb[1].pde = PF2M(pcb[1].pde);
  memset((void *)pcb[1].pde, 0, PGSIZE);
  memcpy((void *)(pcb[1].pde), kpdir, PHY_MEM / PT_SIZE);
  alloc_umem(pcb[1].pde, (uint32_t)pa_to_va(0),
             PGROUNDUP((uint32_t)pa_to_va(sec_num * 512 + 2 * PGSIZE)));
  pcb[1].regs.esp = pcb[1].ustacktop =
      PGROUNDUP((uint32_t)pa_to_va(sec_num * 512 + 2 * PGSIZE));
  pcb[1].start = sec_start;
  pcb[1].sz = sec_num;
  write_cr3(pcb[1].pde);
  init_pool(busy, &_bh, &_bt);
  init_pool(avai, &_ah, &_at);
  init_pool(slp, &_sh, &_st);
  init_pool(zombie, &_zh, &_zt);

  for (int i = 0; i < MAX_PCB_NUM; ++i) {
    pcb_pts[i].data = i;
    insert_tail(avai, pcb_pts + i);
  }
  insert_head(busy, get_proc());
  insert_head(busy, get_proc());

  Log("seg_satrt:%d, sec_num:%d", sec_start, sec_num);
  pcb[1].regs.eip = loadelf(sec_start, sec_num, (uint32_t)pa_to_va(0));
  current = 0;
  asm volatile("movl %0, %%esp" ::"m"(pcb[0].stackTop));
  sti();
  asm volatile("int $0x20");
  idle();
}

/*
kernel is loaded to location 0x100000, i.e., 1MB
size of kernel is not greater than 200*512 bytes, i.e., 100KB
user program is loaded to location 0x200000, i.e., 2MB
size of user program is not greater than 200*512 bytes, i.e., 100KB
*/

//把elf文件加载到Pysaddr开始的地址，即将加载的程序以编号为Secstart的扇区开始，占据Secnum个扇区（LBA），传入的entry是个指针
uint32_t loadelf(uint32_t Secstart, uint32_t Secnum, uint32_t Pysaddr) {
  unsigned i = 0;
  uint32_t entry = 0;
  for (i = 0; i < Secnum; i++) {
    readSect((void *)(elf + i * 512), Secstart + i);
  }
  ELFHeader *eh = (ELFHeader *)elf;
  ProgramHeader *ph = (ProgramHeader *)(elf + eh->phoff);
  ProgramHeader *ph_ass = ph + eh->phnum;
  entry = eh->entry;
  while (ph < ph_ass) {
    if (ph->type == 1) {
      memcpy((void *)(ph->vaddr), ((void *)elf + ph->off), ph->filesz);
      memset((void *)(ph->vaddr + ph->filesz), 0, ph->memsz - ph->filesz);
    }
    ++ph;
  }
  return entry;
}

uint32_t loadUMain() {
  uint32_t entry = loadelf(201, 28, 0);
  
  return entry;
}
