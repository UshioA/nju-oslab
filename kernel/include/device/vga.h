#ifndef __VGA_H__
#define __VGA_H__

void initVga();
void clearScreen();
void updateCursor(int row, int col);
void scrollScreen();
extern int32_t _color;
#endif
