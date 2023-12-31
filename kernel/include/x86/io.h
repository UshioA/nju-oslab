#ifndef __X86_IO_H__
#define __X86_IO_H__
/* ELF32二进制文件头 */
#include <stdint.h>
typedef struct ELFHeader {
  unsigned int magic;
  unsigned char elf[12];
  unsigned short type;
  unsigned short machine;
  unsigned int version;
  unsigned int entry;
  unsigned int phoff;
  unsigned int shoff;
  unsigned int flags;
  unsigned short ehsize;
  unsigned short phentsize;
  unsigned short phnum;
  unsigned short shentsize;
  unsigned short shnum;
  unsigned short shstrndx;
} ELFHeader;

/* ELF32 Program header */
typedef struct ProgramHeader {
  unsigned int type;
  unsigned int off;
  unsigned int vaddr;
  unsigned int paddr;
  unsigned int filesz;
  unsigned int memsz;
  unsigned int flags;
  unsigned int align;
} ProgramHeader;

#define PT_LOAD 1 // Type为LOAD的段才可以被加载
uint32_t loadelf(uint32_t Secstart, uint32_t Secnum, uint32_t Pysaddr);

static inline int inLong(short port) {
  int data;
  asm volatile("in %1, %0" : "=a"(data) : "d"(port));
  return data;
}

/* 读I/O端口 */
static inline uint8_t inByte(uint16_t port) {
  uint8_t data;
  asm volatile("in %1, %0" : "=a"(data) : "d"(port));
  return data;
}

/* 写I/O端口 */
static inline void outByte(uint16_t port, int8_t data) {
  asm volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline void outWord(uint16_t port, int16_t data) {
  asm volatile("out %%ax, %%dx" ::"a"(data), "d"(port));
}

typedef enum {
  NORMAL,
  BACKSPACE,
  NEWLINE,
  NR_keytype,
} keytype;

typedef struct {
  uint32_t code;
  keytype type;
} KEY;

#endif
