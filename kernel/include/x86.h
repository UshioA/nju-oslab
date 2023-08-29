#ifndef __X86_H__
#define __X86_H__

#include "x86/cpu.h"
#include "x86/fs.h"
#include "x86/io.h"
#include "x86/irq.h"
#include "x86/memory.h"
#include "x86/page.h"
#include "x86/pcb.h"

void initSeg(void);
void initProc(void);
void init_page();

#endif
