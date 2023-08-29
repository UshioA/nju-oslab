#ifndef __DEVICE_VIDEO_H__
#define __DEVICE_VIDEO_H__

#include "common.h"
#define VMEM_ADDR ((uint8_t *)0xA0000)
#ifdef GRAPHIC_VGA
#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#define SCR_SIZE ((SCR_WIDTH) * (SCR_HEIGHT))

#define FONT_W 8
#define FONT_H 8
#endif

#ifdef TEXT_VGA
#define SCR_WIDTH 80
#define SCR_HEIGHT 25
#define SCR_SIZE ((SCR_WIDTH) * (SCR_HEIGHT))
#define FONT_H 1
#define FONT_W 1
#endif

extern uint8_t *vmem;

static inline void draw_pixel(int x, int y, int color) {
  assert(x >= 0 && y >= 0 && x < SCR_HEIGHT && y < SCR_WIDTH);
  vmem[x * 320 + y] = color;
}

void prepare_buffer(void);
void display_buffer(void);

void draw_string(const char *, int, int, int);
void draw_character(char ch, int x, int y, int color);
#endif