#ifndef __X86_CPU_H__
#define __X86_CPU_H__

#include "common.h"
#include <stdint.h>

// Eflags register
#define FL_IF 0x00000200 // Interrupt Enable

// Control Register flags
#define CR0_PE 0x00000001 // Protection Enable
#define CR0_WP 0x00010000 // Write Protect
#define CR0_PG 0x80000000 // Paging

#define CR4_PSE 0x00000010 // Page size extension

/* the Control Register 0 */
typedef union CR0 {
  struct {
    uint32_t protect_enable : 1;
    uint32_t monitor_coprocessor : 1;
    uint32_t emulation : 1;
    uint32_t task_switched : 1;
    uint32_t extension_type : 1;
    uint32_t numeric_error : 1;
    uint32_t pad0 : 10;
    uint32_t write_protect : 1;
    uint32_t pad1 : 1;
    uint32_t alignment_mask : 1;
    uint32_t pad2 : 10;
    uint32_t no_write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t paging : 1;
  };
  uint32_t val;
} CR0;

static inline uint32_t read_cr0() {
  uint32_t val;
  asm volatile("movl %%cr0, %0" : "=r"(val));
  return val;
}

/* write CR0 */
static inline void write_cr0(uint32_t cr0) {
  asm volatile("movl %0, %%cr0" : : "r"(cr0));
}

static inline void write_cr3(uint32_t cr3) {
  asm volatile("movl %0, %%cr3" ::"r"(cr3));
}

static inline uint32_t read_eflags(void) {
  uint32_t eflags;
  asm volatile("pushfl; popl %0" : "=r"(eflags));
  return eflags;
}

/* the Control Register 3 (physical address of page directory) */
typedef union CR3 {
  struct {
    uint32_t pad0 : 3;
    uint32_t page_write_through : 1;
    uint32_t page_cache_disable : 1;
    uint32_t pad1 : 7;
    uint32_t page_directory_base : 20;
  };
  uint32_t val;
} CR3;

/* 将CPU置入休眠状态直到下次中断到来 */
static inline void hlt() { asm volatile("hlt"); }

/* 修改IDTR */
static inline void saveIdt(void *addr, uint32_t size) {
  static volatile uint16_t data[3];
  data[0] = size - 1;
  data[1] = (uint32_t)addr;
  data[2] = ((uint32_t)addr) >> 16;
  asm volatile("lidt (%0)" : : "r"(data));
}

/* 打开外部中断 */
static inline void sti(void) { asm volatile("sti"); }

/* 关闭外部中断 */
static inline void cli(void) { asm volatile("cli"); }

#define NR_IRQ 256

#endif
