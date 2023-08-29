#ifndef __X86_MEMORY_H__
#define __X86_MEMORY_H__
#include <stdint.h>
#define DPL_KERN 0
#define DPL_USER 3

// Application segment type bits
#define STA_X 0x8 // Executable segment
#define STA_W 0x2 // Writeable (non-executable segments)
#define STA_R 0x2 // Readable (executable segments)

// System segment type bits
#define STS_T32A 0x9 // Available 32-bit TSS
#define STS_IG32 0xE // 32-bit Interrupt Gate
#define STS_TG32 0xF // 32-bit Trap Gate

// GDT entries
#define NR_SEGMENTS 6 // GDT size
#define SEG_KCODE 1     // Kernel code
#define SEG_KDATA 2     // Kernel data/stack
#define SEG_UCODE 3
#define SEG_UDATA 4
#define SEG_TSS (NR_SEGMENTS - 1)

// Selectors
#define KSEL(desc) (((desc) << 3) | DPL_KERN)
#define USEL(desc) (((desc) << 3) | DPL_USER)

#define PHY_MEM (128 * 1024 * 1024)
#define KERNEL_BASE (0x300000)

#define make_invalid_pde() 0
#define make_invalid_pte() 0
#define make_pde(addr) ((((uint32_t)(addr)) & 0xfffff000) | 0x7)
#define make_pte(addr) ((((uint32_t)(addr)) & 0xfffff000) | 0x7)

//#define KOFFSET 0xc0000000
#define KOFFSET 0x8000000

#define va_to_pa(addr) ((void *)(((uint32_t)(addr)) - KOFFSET))
#define pa_to_va(addr) ((void *)(((uint32_t)(addr)) + KOFFSET))

/* 32bit x86 uses 4KB page size */
#define PAGE_SIZE 4096
#define NR_PDE 1024
#define NR_PTE 1024
#define PAGE_MASK (4096 - 1)
#define PT_SIZE ((NR_PTE) * (PAGE_SIZE))

// page directory index
#define PDX(va) (((uint32_t)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va) (((uint32_t)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES 1024 // # directory entries per page directory
#define NPTENTRIES 1024 // # PTEs per page table
#define PGSIZE 4096     // bytes mapped by a page

#define PTXSHIFT 12 // offset of PTX in a linear address
#define PDXSHIFT 22 // offset of PDX in a linear address

#define PGROUNDUP(sz) (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))

// Page table/directory entry flags.
#define PTE_P 0x001  // Present
#define PTE_W 0x002  // Writeable
#define PTE_U 0x004  // User
#define PTE_PS 0x080 // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte) ((uint32_t)(pte) & ~0xFFF)
#define PTE_FLAGS(pte) ((uint32_t)(pte)&0xFFF)

/* force the data to be aligned with page boundary.
   statically defined page tables uses this feature. */
#define align_to_page __attribute((aligned(PAGE_SIZE)))

/* the 32bit Page Directory(first level page table) data structure */
typedef union PageDirectoryEntry {
  struct {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t page_write_through : 1;
    uint32_t page_cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t pad0 : 6;
    uint32_t page_frame : 20;
  };
  uint32_t val;
} PDE;

/* the 32bit Page Table Entry(second level page table) data structure */
typedef union PageTableEntry {
  struct {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t page_write_through : 1;
    uint32_t page_cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pad0 : 1;
    uint32_t global : 1;
    uint32_t pad1 : 3;
    uint32_t page_frame : 20;
  };
  uint32_t val;
} PTE;

typedef PTE (*PT)[NR_PTE];

struct GateDescriptor {
  uint32_t offset_15_0 : 16;
  uint32_t segment : 16;
  uint32_t pad0 : 8;
  uint32_t type : 4;
  uint32_t system : 1;
  uint32_t privilege_level : 2;
  uint32_t present : 1;
  uint32_t offset_31_16 : 16;
};

struct StackFrame {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
  uint32_t irq, error;
  uint32_t eip, cs, eflags, esp, ss;
};

#define MAX_STACK_SIZE 2048
#define MAX_PCB_NUM 32

#define MAX_TIME_COUNT 16
int32_t find_file(char *target);
/*
1. The number of bits in a bit field sets the limit to the range of values it
can hold
2. Multiple adjacent bit fields are usually packed together (although this
behavior is implementation-defined)

Refer: en.cppreference.com/w/cpp/language/bit_field
*/
struct SegDesc {
  uint32_t lim_15_0 : 16;  // Low bits of segment limit
  uint32_t base_15_0 : 16; // Low bits of segment base address
  uint32_t base_23_16 : 8; // Middle bits of segment base address
  uint32_t type : 4;       // Segment type (see STS_ constants)
  uint32_t s : 1;          // 0 = system, 1 = application
  uint32_t dpl : 2;        // Descriptor Privilege Level
  uint32_t p : 1;          // Present
  uint32_t lim_19_16 : 4;  // High bits of segment limit
  uint32_t avl : 1;        // Unused (available for software use)
  uint32_t rsv1 : 1;       // Reserved
  uint32_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  uint32_t g : 1;          // Granularity: limit scaled by 4K when set
  uint32_t base_31_24 : 8; // High bits of segment base address
};
typedef struct SegDesc SegDesc;

#define SEG(type, base, lim, dpl)                                              \
  (SegDesc) {                                                                  \
    ((lim) >> 12) & 0xffff, (uint32_t)(base)&0xffff,                           \
        ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,                      \
        (uint32_t)(lim) >> 28, 0, 0, 1, 1, (uint32_t)(base) >> 24              \
  }

#define SEG16(type, base, lim, dpl)                                            \
  (SegDesc) {                                                                  \
    (lim) & 0xffff, (uint32_t)(base)&0xffff, ((uint32_t)(base) >> 16) & 0xff,  \
        type, 0, dpl, 1, (uint32_t)(lim) >> 16, 0, 0, 1, 0,                    \
        (uint32_t)(base) >> 24                                                 \
  }

// Task state segment format
struct TSS {
  uint32_t link; // old ts selector
  uint32_t esp0; // Ring 0 Stack pointer and segment selector
  uint32_t ss0;  // after an increase in privilege level
  union {
    struct {
      char dontcare[88];
    };
    struct {
      uint32_t esp1, ss1, esp2, ss2;
      uint32_t cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
      uint32_t es, cs, ss, ds, fs, gs, ldt;
    };
  };
};
typedef struct TSS TSS;

static inline void setGdt(SegDesc *gdt, uint32_t size) {
  volatile static uint16_t data[3];
  data[0] = size - 1;
  data[1] = (uint32_t)gdt;
  data[2] = (uint32_t)gdt >> 16;
  asm volatile("lgdt (%0)" : : "r"(data));
}

static inline void lLdt(uint16_t sel) { asm volatile("lldt %0" ::"r"(sel)); }

#endif
