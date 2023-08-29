#include "device.h"
#include "x86.h"
#include <stdint.h>

int displayRow = 0;
int displayCol = 0;
uint16_t displayMem[80 * 25];
int displayClear = 0;
int32_t _color = 0x0c;

void initVga() {
  displayRow = 0;
  displayCol = 0;
  displayClear = 0;
  _color = 0x0c;
#ifdef TEXT_VGA
  clearScreen();
  updateCursor(0, 0);
#endif
}

void clearScreen() {
  int i = 0;
  int pos = 0;
  uint16_t data = 0 | (0x0c << 8);
  for (i = 0; i < 80 * 25; i++) {
    pos = i * 2;
    asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
  }
}

void updateCursor(int row, int col) {
  int cursorPos = row * 80 + col;
  outByte(0x3d4, 0x0f);
  outByte(0x3d5, (unsigned char)(cursorPos & 0xff));

  outByte(0x3d4, 0x0e);
  outByte(0x3d5, (unsigned char)((cursorPos >> 8) & 0xff));
}

void scrollScreen() {
#ifdef TEXT_VGA
  int i = 0;
  int pos = 0;
  uint16_t data = 0;
  for (i = 0; i < 80 * 25; i++) {
    pos = i * 2;
    asm volatile("movw (%1), %0" : "=r"(data) : "r"(pos + 0xb8000));
    displayMem[i] = data;
  }
  for (i = 0; i < 80 * 24; i++) {
    pos = i * 2;
    data = displayMem[i + 80];
    asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
  }
  data = 0 | (_color << 8);
  for (i = 80 * 24; i < 80 * 25; i++) {
    pos = i * 2;
    asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
  }
#endif

#ifdef GRAPHIC_VGA
  memmove(vmem, vmem + 8 * SCR_WIDTH * sizeof(uint8_t),
          (SCR_SIZE) * sizeof(uint8_t));
#endif
}

#include "common.h"
#include "device.h"

extern char font6x8_ascii[128][6];
extern char font8x8_basic[128][8];

// static uint8_t vbuf[SCR_SIZE];
uint8_t *vmem = (void *)VMEM_ADDR;

void prepare_buffer(void) { memset(vmem, 0, SCR_SIZE); }

void display_buffer(void) {
  asm volatile("cld; rep movsl"
               :
               : "c"(SCR_SIZE / 4), "S"(vmem), "D"(VMEM_ADDR));
}

void draw_character(char ch, int x, int y, int color) {
#ifdef GRAPHIC_VGA
  int i, j;
  assert((ch & 0x80) == 0);
  char *p = font8x8_basic[(int)ch];
  for (i = 0; i < FONT_H; i++) {
    for (j = 0; j < FONT_W; j++) {
      if (p[i] >> j & 1) {
        draw_pixel(x + i, y + j, color);
      }
    }
  }
#endif

#ifdef TEXT_VGA
  int pos = (x * SCR_WIDTH + y)<<1;
  asm volatile("movw %0, (%1)" ::"r"((uint16_t)ch | (_color << 8)),
               "r"(pos + 0xb8000));

#endif
}

void draw_string(const char *str, int x, int y, int color) {
  while (*str) {
    draw_character(*str++, x, y, color);
    if (y + FONT_W >= SCR_WIDTH) {
      x += FONT_H;
      y = 0;
    } else {
      y += FONT_H;
    }
  }
}