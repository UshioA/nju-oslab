#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>
void initSerial(void);
void putChar(char);
void putS(char *);
void putNum(int num);
int d2s(int decimal, char *buffer, int size, int count);
int h2s(uint32_t hexadecimal, char *buffer, int size, int count);
int s2s(char *string, char *buffer, int size, int count);
#define MAX_BUFFER_SIZE 256
void print(const char *format, ...);
void shutdown();
#define SERIAL_PORT 0x3F8

#endif
