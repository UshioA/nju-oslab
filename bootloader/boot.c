#include "boot.h"
#include <stdint.h>

#define SECTSIZE 512
#define PH_LOAD 1 // Type为LOAD的段才可以被加载

static void *memcpy(void *dst, void *src, unsigned len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    *((unsigned char *)(dst + i)) = *((unsigned char *)(src + i));
  }
  return dst;
}

static void *memset(void *dst, unsigned char ch, unsigned len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    *((unsigned char *)(dst + i)) = ch;
  }
  return dst;
}

void bootMain(void) {
  ELFHeader *elf = (void *)0x300000;
  ProgramHeader *ph, *ph_ass;
  uint32_t entry;
  for (int i = 0; i < 200; ++i)
    readSect((void *)((uint32_t)elf + i * 512), 1 + i);
  ph = (void *)((uint32_t)elf + elf->phoff);
  ph_ass = ph + elf->phnum;
  entry = elf->entry;
  for (; ph < ph_ass; ++ph) {
    if (ph->type == PH_LOAD) {
      uint32_t va = ph->vaddr;
      memcpy((void *)va, (void *)((uint32_t)elf + ph->off), ph->filesz);
      memset((void *)(va + ph->filesz), 0, ph->memsz - ph->filesz);
    }
  }
  ((void (*)())entry)();
}

void waitDisk(void) { // waiting for disk
  while ((inByte(0x1F7) & 0xC0) != 0x40)
    ;
}

void readSect(void *dst, int offset) { // reading a sector of disk
  int i;
  waitDisk();
  outByte(0x1F2, 1);
  outByte(0x1F3, offset);
  outByte(0x1F4, offset >> 8);
  outByte(0x1F5, offset >> 16);
  outByte(0x1F6, (offset >> 24) | 0xE0);
  outByte(0x1F7, 0x20);

  waitDisk();
  for (i = 0; i < SECTSIZE / 4; i++) {
    ((int *)dst)[i] = inLong(0x1F0);
  }
}
