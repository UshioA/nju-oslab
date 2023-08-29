#include "common.h"
#include "common/off.h"
#include "device.h"
#include "device/keyboard.h"
#include "device/vedio.h"
#include "semaphore.h"
#include "socket.h"
#include "x86.h"
#include "x86/cpu.h"
#include "x86/io.h"

void kEntry(void) {

  // Interruption is disabled in bootloader
  cli();
  initSerial(); // initialize serial port
  initKeyTable();
  initIdt();  // initialize idt
  initIntr(); // iniialize 8259a
  initSeg();  // initialize gdt, tss
  init_page();
  init_sem();
  init_dev();
  init_socket();
  initVga();   // initialize vga device
  initTimer(); // initialize timer device
  initProc();
}
