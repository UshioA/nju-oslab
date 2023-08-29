#include "lib.h"
#include "types.h"
#include "znrlib.h"
/*
 * io lib here
 * 库函数写在这
 */

#define isspace(a) ((a) == ' ' || (a) == '\t')
#define incre(head)                                                            \
  do {                                                                         \
    if ((head) == 15)                                                          \
      (head) = 0;                                                              \
    else                                                                       \
      ++(head);                                                                \
  } while (0)

#define skip_whitespace(buf)                                                   \
  do {                                                                         \
    while (isspace((*buf)))                                                    \
      ++(buf);                                                                 \
  } while (0)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) + (b)-max((a), (b)))

// static inline int32_t syscall(int num, uint32_t a1,uint32_t a2,
int32_t syscall(int num, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                uint32_t a5) {
  int32_t ret = 0;
  // Generic system call: pass system call number in AX
  // up to five parameters in DX,CX,BX,DI,SI
  // Interrupt kernel with T_SYSCALL
  //
  // The "volatile" tells the assembler not to optimize
  // this instruction away just because we don't use the
  // return value
  //
  // The last clause tells the assembler that this can potentially
  // change the condition and arbitrary memory locations.

  /*
   Note: ebp shouldn't be flushed
      May not be necessary to store the value of eax, ebx, ecx, edx, esi, edi
  */
  uint32_t eax, ecx, edx, ebx, esi, edi;

  asm volatile("movl %%eax, %0" : "=m"(eax)); // store general registers' value
  asm volatile("movl %%ecx, %0" : "=m"(ecx));
  asm volatile("movl %%edx, %0" : "=m"(edx));
  asm volatile("movl %%ebx, %0" : "=m"(ebx));
  asm volatile("movl %%esi, %0" : "=m"(esi));
  asm volatile("movl %%edi, %0" : "=m"(edi));

  asm volatile("movl %0, %%eax" ::"m"(num)); // move parameters into register
  asm volatile("movl %0, %%ecx" ::"m"(a1));
  asm volatile("movl %0, %%edx" ::"m"(a2));
  asm volatile("movl %0, %%ebx" ::"m"(a3));
  asm volatile("movl %0, %%esi" ::"m"(a4));
  asm volatile("movl %0, %%edi" ::"m"(a5));

  asm volatile("int $0x80");                  // int instrument,interrupt
  asm volatile("movl %%eax, %0" : "=m"(ret)); // treat eax as return value

  asm volatile("movl %0, %%eax" ::"m"(eax)); // restore general register's value
  asm volatile("movl %0, %%ecx" ::"m"(ecx));
  asm volatile("movl %0, %%edx" ::"m"(edx));
  asm volatile("movl %0, %%ebx" ::"m"(ebx));
  asm volatile("movl %0, %%esi" ::"m"(esi));
  asm volatile("movl %0, %%edi" ::"m"(edi));

  return ret;
}

char getc() { // 对应SYS_READ STD_IN
  return syscall(SYS_READ, STD_IN, (uint32_t)114514, 1, 0, 0);
}

void putc(char c) {
  syscall(SYS_WRITE, STD_OUT, (uint32_t)&c, (uint32_t)1, 0, 0);
}

void puts(const char *s) {
  while (*s)
    putc(*(s++));
}

int scanf(char const *fmt, ...) {
  static const char *input_buf[0xff]; 
  char state = 0;                     // 0 for scanning, 1 for matching
  int ret = 0;
  va_list vp;
  char *format = (void *)fmt;
  va_start(vp, fmt);
get_parameter:; 
  char *buf = (void *)input_buf;
  int re = getStr(buf, 0xfff);
  if (re == -1)
    return re;
  while (*format && *buf) {
    skip_whitespace(buf);
    skip_whitespace(format);
    if (state == 0) {
      if (*format == *buf) {
        ++format;
        ++buf;
      } else {
        if (*(format) == '%') {
          ++format;
          state = 1;
        } else
          goto oh_shit; 
      }
    } else { 
      int len = 0;
      char c = *format;
      ++format;
      skip_whitespace(format);
      switch (c) {
      case 'd': {
        *va_arg(vp, int *) = atoi(buf, 10, &len);
        buf += len;
        ++ret;
        break;
      }
      case 'x': {
        if (*buf == '0')
          ++buf;
        else
          goto oh_shit;
        if (*buf == 'x' || *buf == 'X')
          ++buf;
        else
          goto oh_shit;
        *va_arg(vp, int *) = atoi(buf, 16, &len);
        buf += len;
        ++ret;
        break;
      }
      case 's': {
        char *target = va_arg(vp, char *);
        while (*buf && !isspace(*buf) && *buf != *format) {
          *(target++) = *(buf++);
        }
        *(target++) = 0;
        ++ret;
        break;
      }
      case 'c': {
        *va_arg(vp, char *) = *(buf++);
        ++ret;
        break;
      }
      default:
        goto oh_shit;
      }
      state = 0;
    }
  }
  if (*format)
    goto get_parameter;
  va_end(vp);
  return ret;
oh_shit:
  return buf - (char *)input_buf + 1;
}

char getchar() {
  char c;
  // TODO: 实现getChar函数，方式不限
  do {
    c = getc();
  } while (c == 0x7f);
  printf("%c", c);
  printf("%c",
         getc()); 
                  
                  
                  
  return c;
}

int getStr(char *str, int size) { // 对应SYS_READ STD_STR
  // TODO: 实现getStr函数，方式不限
  unsigned index = 0;
  while (index < size) {
    char c = getc();
    if (c == -1)
      return -1;
    if (c == 0x7f) {
      if (index > 0) {
        printf("\b \b");
        str[index--] = 0;
      }
    } else if (c == '\n') {
      printf("\n");
      break;
    } else {
      printf("%c", c);
      str[index++] = c;
    }
  }
  str[index] = '\0';
  return 0;
}

int dec2Str(int decimal, char *buffer, int size, int count);
int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count);
int str2Str(char *string, char *buffer, int size, int count);

int printf(const char *format, ...) {
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
        int tmp = dec2Str(decimal, buffer, MAX_BUFFER_SIZE, count - 2);
        if (tmp - count < 2) {
          buffer[tmp] = 0;
        }
        count = tmp;
        state = 0;
      } else if (character == 'x') {
        hexadecimal = *((uint32_t *)paraList);
        paraList = (uint32_t *)paraList + 1;
        int tmp = hex2Str(hexadecimal, buffer, MAX_BUFFER_SIZE, count - 2);
        if (tmp - count < 2) {
          buffer[tmp] = 0;
        }
        count = tmp;
        state = 0;
      } else if (character == 's') {
        string = *((char **)paraList);
        paraList = (char **)paraList + 1;
        int tmp = str2Str(string, buffer, MAX_BUFFER_SIZE, count - 2);
        if (tmp - count < 2) {
          buffer[tmp] = 0;
        }
        count = tmp;
        state = 0;
      } else if (character == 'c') {
        buffer[--count] = 0;
        character = *((char *)paraList);
        paraList = (char *)paraList + 4;
        if (!count)
          return -1;
        buffer[count - 1] = character;
        state = 0;
      } else {
        state = 2;
      }
      break;
    }
    case 2: {
      return -2;
    }
    }
  }
  if (count != 0)
    puts(buffer);
  return 0;
}

int dec2Str(int decimal, char *buffer, int size, int count) {
  int i = 0;
  int temp;
  int number[16];

  if (decimal < 0) {
    buffer[count] = '-';
    count++;
    if (count == size) {
      puts(buffer);
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
      puts(buffer);
      count = 0;
    }
    i--;
  }
  return count;
}

int hex2Str(uint32_t hexadecimal, char *buffer, int size, int count) {
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
      puts(buffer);
      count = 0;
    }
    i--;
  }
  return count;
}

int str2Str(char *string, char *buffer, int size, int count) {
  int i = 0;
  while (string[i] != 0) {
    buffer[count] = string[i];
    count++;
    if (count == size) {
      puts(buffer);
      count = 0;
    }
    i++;
  }
  return count;
}

pid_t fork() { return syscall(SYS_FORK, SYS_FORK, 0, 0, 0, 0); }

pid_t thread_create(void (*func)(void *), uint32_t argc, void *args) {
  return syscall(SYS_FORK, SYS_THREAD, (uint32_t)func, argc, (uint32_t)args, 0);
}

// here just reuse sys_wait😋
pid_t thread_join(pid_t pid) {
  uint32_t status;
  syscall(SYS_WAIT, (uint32_t)&status, pid, 0, 0, 0);
  return status;
}

int thread_start(pid_t pid) { return syscall(THREAD_START, pid, 0, 0, 0, 0); }

int exec(char *name, int argc, char **argv) {
  syscall(SYS_EXEC, (uint32_t)name, (uint32_t)argc, (uint32_t)argv, 0, 0);
  return 0;
}

int sleep(uint32_t time) {
  syscall(SYS_SLEEP, time, 0, 0, 0, 0);
  return 0;
}

int exit(int32_t code) {
  syscall(SYS_EXIT, code, 0, 0, 0, 0);
  return 0;
}

int trap() {
  syscall(SYS_TRAP, 0, 0, 0, 0, 0);
  return 0;
}

int wait(int32_t *status) {
  return syscall(SYS_WAIT, (uint32_t)status, -1, 0, 0, 0);
}

sem_t sem_init(int32_t value) {
  return syscall(SYS_SEM, SEM_INIT, value, 0, 0, 0);
}

int sem_wait(sem_t s) { return syscall(SYS_SEM, SEM_P, (uint32_t)s, 0, 0, 0); }

int sem_post(sem_t s) { return syscall(SYS_SEM, SEM_V, (uint32_t)s, 0, 0, 0); }

int sem_destroy(sem_t s) {
  return syscall(SYS_SEM, SEM_DESTROY, (uint32_t)s, 0, 0, 0);
}

int getpid() { return syscall(SYS_PID, 0, 0, 0, 0, 0); }

sock_t socket_init() { return syscall(SYS_SOCKET, SOCK_NEW, 0, 0, 0, 0); }

void socket_destroy(sock_t port) {
  syscall(SYS_SOCKET, SOCK_DESTROY, port, 0, 0, 0);
}
void __socket_send(udp *msg) {
  syscall(SYS_SOCKET, SOCK_SEND, (uint32_t)msg, 0, 0, 0);
}
int __socket_recv(sock_t port, udp *recv) {
  return syscall(SYS_SOCKET, SOCK_RECV, port, (uint32_t)recv, 0, 0);
}