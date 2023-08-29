#include "device.h"
#include "x86.h"
#include "x86/io.h"

void initSerial(void) {
  outByte(SERIAL_PORT + 1, 0x00);
  outByte(SERIAL_PORT + 3, 0x80);
  outByte(SERIAL_PORT + 0, 0x01);
  outByte(SERIAL_PORT + 1, 0x00);
  outByte(SERIAL_PORT + 3, 0x03);
  outByte(SERIAL_PORT + 2, 0xC7);
  outByte(SERIAL_PORT + 4, 0x0B);
}

static inline int serialIdle(void) {
  return (inByte(SERIAL_PORT + 5) & 0x20) != 0;
}

void shutdown(void) {
  Say("Good Night😇");
  outWord(0x604, 0x2000);
}

void putChar(char ch) {
  while (serialIdle() != 1)
    ;
  outByte(SERIAL_PORT, ch);
}

void putS(char *ch) {
  while (ch && (*ch) && (*ch) != '\0') {
    putChar(*ch);
    ch++;
  }
}

void putNum(int num) { print("%d", num); }

int d2s(int decimal, char *buffer, int size, int count);
int h2s(uint32_t hexadecimal, char *buffer, int size, int count);
int s2s(char *string, char *buffer, int size, int count);
void print(const char *format, ...) {
  int i = 0; // format index
  char buffer[MAX_BUFFER_SIZE];
  for (int i = 0; i < MAX_BUFFER_SIZE; ++i) {
    buffer[i] = 0;
  }
  int count = 0; // buffer index
  // int index = 0;                    // parameter index
  void *paraList = (void *)&format; // address of format in stack
  int state = 0; // 0: legal character; 1: '%'; 2: illegal format
  int decimal = 0;
  uint32_t hexadecimal = 0;
  char *string = 0;
  char character = 0;
  // void *para = 0;
  paraList = (char **)paraList + 1;
  while (format[i] != 0) {
    character = buffer[count++] = format[i++];
    // TODO: 可以借助状态机（回忆数电），辅助的函数已经实现好了，注意阅读手册
    switch (state) {
    case 0: {
      if (character == '%') {
        state = 1;
      }
      break;
    }
    case 1: {
      if (character == '%') {
        state = 0;
        buffer[count--] = 0;
      } else if (character == 'd') { // integer
        decimal = *((int32_t *)paraList);
        paraList = (int32_t *)paraList + 1;
        count = d2s(decimal, buffer, MAX_BUFFER_SIZE, count - 2);
        state = 0;
      } else if (character == 'x') {
        hexadecimal = *((uint32_t *)paraList);
        paraList = (uint32_t *)paraList + 1;
        count = h2s(hexadecimal, buffer, MAX_BUFFER_SIZE, count - 2);
        state = 0;
      } else if (character == 's') {
        string = *((char **)paraList);
        paraList = (char **)paraList + 1;
        count = s2s(string, buffer, MAX_BUFFER_SIZE, count - 2);
        state = 0;
      } else if (character == 'c') {
        buffer[--count] = 0;
        character = *((char *)paraList);
        paraList = (char *)paraList + 4;
        if (!count)
          return;
        buffer[count - 1] = character;
        state = 0;
      } else {
        state = 2;
      }
      break;
    }
    case 2: {
      return;
    }
    }
  }
  if (count != 0)
    putS(buffer);
}

int d2s(int decimal, char *buffer, int size, int count) {
  int i = 0;
  int temp;
  int number[16];

  if (decimal < 0) {
    buffer[count] = '-';
    count++;
    if (count == size) {
      putS(buffer);
      count = 0;
    }
    temp = decimal / 10;
    number[i] = temp * 10 - decimal;
    decimal = temp;
    i++;
    while (decimal != 0) {
      temp = decimal / 10;
      number[i] = temp * 10 - decimal;
      decimal = temp;
      i++;
    }
  } else {
    temp = decimal / 10;
    number[i] = decimal - temp * 10;
    decimal = temp;
    i++;
    while (decimal != 0) {
      temp = decimal / 10;
      number[i] = decimal - temp * 10;
      decimal = temp;
      i++;
    }
  }

  while (i != 0) {
    buffer[count] = number[i - 1] + '0';
    count++;
    if (count == size) {
      putS(buffer);
      count = 0;
    }
    i--;
  }
  return count;
}

int h2s(uint32_t hexadecimal, char *buffer, int size, int count) {
  int i = 0;
  uint32_t temp = 0;
  int number[16];

  temp = hexadecimal >> 4;
  number[i] = hexadecimal - (temp << 4);
  hexadecimal = temp;
  i++;
  while (hexadecimal != 0) {
    temp = hexadecimal >> 4;
    number[i] = hexadecimal - (temp << 4);
    hexadecimal = temp;
    i++;
  }

  while (i != 0) {
    if (number[i - 1] < 10)
      buffer[count] = number[i - 1] + '0';
    else
      buffer[count] = number[i - 1] - 10 + 'a';
    count++;
    if (count == size) {
      putS(buffer);
      count = 0;
    }
    i--;
  }
  return count;
}

int s2s(char *string, char *buffer, int size, int count) {
  int i = 0;
  while (string[i] != 0) {
    buffer[count] = string[i];
    count++;
    if (count == size) {
      putS(buffer);
      count = 0;
    }
    i++;
  }
  return count;
}
