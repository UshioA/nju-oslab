#include "x86/page.h"
#include "common.h"
#include "common/assert.h"
#include "common/debug.h"
#include "common/off.h"
#include "device.h"
#include "device/disk.h"
#include "x86.h"
#include "x86/memory.h"
#include <stdint.h>

byte pagemap[(PHY_MEM - KERNEL_BASE) / (PGSIZE*8)]; // reserve address under kernel_base
extern TSS tss;
void set_page(uint32_t index, char value) {
  if (value) {
    set_bit(index >> 3, index % 8);
  } else {
    clear_bit(index >> 3, index % 8);
  }
}

void toggle_page(uint32_t index) { toggle_bit(index >> 3, index % 8); }

int32_t kalloc() {
  for (int i = 0; i < (PHY_MEM - KERNEL_BASE) / (PGSIZE*8); ++i) {
    for (int j = 0; j < 8; ++j) {
      if (!peek_bit(i, j)) {
        toggle_page(j + (i << 3));
        return ((j + (i << 3)));
      }
    }
  }
  return -1;
}

void kfree(uint32_t addr) {
  // Assert(addr < PHY_MEM && addr > 0x200000, "傻逼你地址越界了");
  memset((uint8_t *)addr, 1, PAGE_SIZE);
  toggle_page(M2PF(addr));
}

int32_t alloc_umem(uint32_t pgdir, uint32_t oldsz, uint32_t newsz) {
  uint8_t *mem;
  for (int i = oldsz; i < newsz; i += PAGE_SIZE) {
    mem = (void *)kalloc();
    if ((int)mem == -1) {
      Log("alloc_umem out of memory:(");
      deallocuvm((void *)pgdir, newsz, oldsz);
      return 0;
    }
    mem = (void *)PF2M(mem);
    memset(mem, 0, PAGE_SIZE);
    if (mappages((void *)pgdir, (void *)i, PAGE_SIZE, (uint32_t)mem,
                 PTE_W | PTE_U) < 0) {
      Log("alloc_umem out of memory (2) :(");
      // TODO dealloc
      deallocuvm((void *)pgdir, newsz, oldsz);
      kfree((uint32_t)mem);
      return 0;
    }
  }
  // Log("complete");
  return newsz;
}

int32_t mappages(PDE *pgdir, void *va, uint32_t size, uint32_t pa, int perm) {
  char *a, *last;
  PTE *pte;
  a = (char *)PGROUNDDOWN((uint32_t)va);
  last = (char *)PGROUNDDOWN((uint32_t)va + size - 1);
  while (1) {
    if (((uint32_t)(pte = walkpgdir(pgdir, a, 1)) == 0x100000)) {
      return -1;
    }
    Assert(!pte->present, "a : 0x%x", a);
    pte->val = pa | perm | PTE_P;
    // Log("mem : 0x%x", pa);
    // Log("pte : 0x%x , page_frame : 0x%x", pte, pte->page_frame);
    if (a == last) {
      break;
    }
    a += PAGE_SIZE;
    pa += PAGE_SIZE;
  }
  return 0;
}

PTE *walkpgdir(PDE *pgdir, const void *va, int alloc) {
  PDE *pde;
  PTE *pgtab;
  // Log("va dir : 0x%x", va_u.dir);
  pde = pgdir + PDX(va);
  if (pde->present) {
    // Log("pde present, val, 0x%x", pde->val);
    pgtab = (PTE *)PTE_ADDR(pde->val);
  } else {
    if (!alloc || ((int)(pgtab = (PTE *)kalloc()) == -1)) {
      return (void *)0x100000;
    }
    pgtab = (void *)PF2M(pgtab);
    memset(pgtab, 0, PAGE_SIZE);
    pde->val = make_pde(pgtab);
    // Log("pde not present , val, 0x%x", pde->val);
  }
  // Log("pgtab : 0x%x", pgtab);
  // Log("return : 0x%x", pgtab + (va_u.page << 12));
  return pgtab + PTX(va);
}

void switchuvm(struct ProcessTable *p) {
  Assert(p->pde, "怎么回事呢");
  pushoff();
  tss.esp0 = p->stackTop;
  write_cr3(p->pde);
  popoff();
}

extern PDE kpdir[NR_PDE] align_to_page;

PDE *copyuvm(PDE *pgdir, uint32_t size) {
  PDE *d;
  PTE *pte;
  uint32_t pa, i, flags;
  char *mem;
  d = (void *)kalloc();
  d = (void *)PF2M(d);
  memset(d, 0, PAGE_SIZE);
  memcpy(d, kpdir, PHY_MEM / PT_SIZE);
  for (i = (uint32_t)pa_to_va(0); i < (uint32_t)pa_to_va(size); i += PGSIZE) {
    Assert((int32_t)(pte = walkpgdir(pgdir, (void *)i, 0)) != 0x100000,
           "怎么会是呢");
    Assert(pte->present, "原来的分配有误啊");

    pa = PTE_ADDR(pte->val);
    flags = PTE_FLAGS(pte->val);
    if ((int32_t)((mem = (void *)kalloc())) == -1) {
      kfree((uint32_t)mem);
      return 0;
    }
    mem = (void *)PF2M(mem);
    // Log("pa : 0x%x to mem : 0x%x", pa, mem);
    memcpy(mem, (uint8_t *)pa, PAGE_SIZE);
    if (mappages(d, (void *)i, PAGE_SIZE, (uint32_t)mem, flags) < 0) {
      kfree((uint32_t)mem);
      return 0;
    }
  }
  return d;
}

int deallocuvm(PDE *pgdir, uint32_t oldsz, uint32_t newsz) {
  PTE *pte;
  uint32_t a, pa;
  if (newsz >= oldsz) {
    return oldsz;
  }

  a = PGROUNDUP(newsz);
  for (; a < oldsz; a += PGSIZE) {
    pte = walkpgdir(pgdir, (uint8_t *)a, 0);
    if ((uint32_t)pte == 0x100000)
      a = PGADDR(PDX(a) + 1, 0, 0);
    else if (pte->present) {
      pa = PTE_ADDR(pte->val);
      assert(pa);
      kfree(pa);
      pte->val = 0;
    }
  }
  return newsz;
}

void freevm(PDE *pgdir, uint32_t oldsz, uint32_t newsz) {
  uint32_t i;

  Assert(pgdir, "我页目录呢");

  deallocuvm(pgdir, oldsz, newsz);
  for (i = 0; i < NPDENTRIES; i++) {
    if (pgdir[i].val & PTE_P) {
      char *v = (void *)PTE_ADDR(pgdir[i].val);
      kfree((uint32_t)v);
    }
  }
  kfree((uint32_t)pgdir);
}

// int loaduvm(PDE *pgdir, char *addr, uint32_t offset, uint32_t size) {
//   uint32_t i, pa, n;
//   PTE *pte;

//   Assert(!((uint32_t)addr % PAGE_SIZE), "addr must be page aligned");

//   for (i = 0; i < size; i += PAGE_SIZE) {
//     if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0)
//       Assert(0, "address not exist");
//     pa = pte->page_frame << 12;
//     if (size - i < PGSIZE) {
//       n = size - i;
//     } else {
//       n = PAGE_SIZE;
//     }
//     for (int j = 0; j < n / 512; ++j) {
//       readSect((void *)pa + j * 512, j + offset + i / 512);
//     }
//   }
//   return 0;
// }