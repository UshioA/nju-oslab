#ifndef __FS__
#define __FS__
#include "common.h"
#include <stdint.h>
typedef struct file_info {
  char name[100];
  uint32_t sec_start;
  uint32_t sec_len;
} file_info;

extern file_info file_info_table[];

#define NR_FILE sizeof(file_info_table) / sizeof(file_info)
#endif