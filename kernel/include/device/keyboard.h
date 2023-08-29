#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define MAX_KEYBUFFER_SIZE 256

void initKeyTable();

unsigned getKeyCode();

char getChar(unsigned code);

#endif