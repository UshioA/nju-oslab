#ifndef __lib_h__
#define __lib_h__

#include "types.h"
#include "udp.h"
#include <stdarg.h>

#define SYS_WRITE 0
#define SYS_FORK 1
#define SYS_EXEC 2
#define SYS_SLEEP 3
#define SYS_EXIT 4
#define SYS_READ 5
#define SYS_WAIT 6
#define SYS_TRAP 7
#define THREAD_START 8
#define SYS_SEM 9
#define SYS_PID 11
#define SYS_SOCKET 12

#define STD_OUT 0
#define STD_IN 1
#define STD_STR 2

#define SEM_INIT 0
#define SEM_P 1
#define SEM_V 2
#define SEM_DESTROY 3

#define SOCK_NEW 0
#define SOCK_DESTROY 1
#define SOCK_SEND 2
#define SOCK_RECV 3

#define MAX_BUFFER_SIZE 256

#define SYS_THREAD 0

int printf(const char *format, ...);
char getchar();
int getStr(char *str, int size);
int scanf(char const *fmt, ...);

char getc();
void putc(char c);
void puts(const char *s);

pid_t fork();

int exec(char *, int, char **);

int sleep(uint32_t time);

int exit(int32_t code);

int wait(int32_t *status);

int trap();

pid_t thread_create(void (*func)(void *), uint32_t, void *args);

pid_t thread_join(pid_t pid);

int thread_start(pid_t pid);

sem_t sem_init(int32_t);
int sem_wait(sem_t);
int sem_post(sem_t);
int sem_destroy(sem_t);

int getpid();

sock_t socket_init();
void socket_destroy(sock_t port);
void __socket_send(udp *msg);
int __socket_recv(sock_t port, udp *recv);

#endif
