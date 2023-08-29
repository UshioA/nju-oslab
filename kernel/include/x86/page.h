#ifndef __PAGE__
#define __PAGE__
#include "x86/memory.h"
#include "x86/pcb.h"
#include <stdint.h>

#define set_bit(b, i)                                                          \
  do {                                                                         \
    pagemap[(b)].val |= 1 << (i);                                              \
  } while (0)

#define toggle_bit(b, i)                                                       \
  do {                                                                         \
    pagemap[(b)].val ^= 1 << (i);                                              \
  } while (0)

#define clear_bit(b, i)                                                        \
  do {                                                                         \
    pagemap[(b)].val &= ~(1 << (i));                                           \
  } while (0)

#define peek_bit(b, i) ((pagemap[(b)].val >> (i)) & 0x01)

#define PF2M(mem) ((((uint32_t)(mem)) << 12) + KERNEL_BASE)
#define M2PF(mem) ((((uint32_t)(mem)) - KERNEL_BASE) >> 12)

typedef union {
  uint32_t val;
  struct {
    uint32_t offset : 12;
    uint32_t page : 10;
    uint32_t dir : 10;
  };
} laddr_u;

typedef union byte {
  struct {
    uint8_t _1 : 1;
    uint8_t _2 : 1;
    uint8_t _3 : 1;
    uint8_t _4 : 1;
    uint8_t _5 : 1;
    uint8_t _6 : 1;
    uint8_t _7 : 1;
    uint8_t _8 : 1;
  };
  uint8_t val;
} byte;

void set_page(uint32_t index, char value);
void toggle_page(uint32_t index);
void kfree(uint32_t addr);
int32_t kalloc();
int32_t mappages(PDE *pgdir, void *va, uint32_t size, uint32_t pa, int perm);
PTE *walkpgdir(PDE *pgdir, const void *va, int alloc);
int32_t alloc_umem(uint32_t pgdir, uint32_t oldsz, uint32_t newsz);
void switchuvm(struct ProcessTable *pcb);
void inituvm(PDE *pgdir, char *init, uint32_t sz);
PDE *copyuvm(PDE *pgdir, uint32_t sz);
void freevm(PDE *pgdir, uint32_t oldsz, uint32_t newsz);
int deallocuvm(PDE *pgdir, uint32_t oldsz, uint32_t newsz);
extern byte pagemap[(PHY_MEM-KERNEL_BASE)/(PGSIZE * 8)];
#endif