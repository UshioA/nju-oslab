#ifndef __DEBUG__H__
#define __DEBUG__H__
#include "common/off.h"
#include "config.h"
#include "device/serial.h"
#ifdef DEBUG
#define Log(a, ...)                                                            \
  do {                                                                         \
    pushoff();                                                                 \
    print("\e[35m{%s:%d}\e[0m:" a "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
    popoff();                                                                  \
  } while (0)
#else
#define Log(a, ...)                                                            \
  do {                                                                         \
  } while (0)
#endif

#define Say(a, ...)                                                            \
  do {                                                                         \
    print("\e[32m{%s:%d}\e[0m:" a "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
  } while (0)
#endif