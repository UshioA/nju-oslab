#include "lib.h"
#include "socket.h"
#include "types.h"
#include "znrlib.h"

char ho[] = "hello, world!\0";

sem_t dollar;

typedef void (*vfunc)(void);

void func_wrapper(vfunc f){
  pid_t pid = fork();
  if(pid >= 0){
    if(pid == 0){
      f();
      exit(0);
    }else{
      while(wait(0)==-1);
    }
  }
}

void forktest(void) {
  int n, pid;

  printf("fork test\n");

  for (n = 0; n < 114; n++) {
    pid = fork();
    if (pid < 0)
      break;
    if (pid == 0)
      exit(0);
  }

  if (n == 114) {
    printf("fork claimed to work 10 times!");
    exit(-1);
  }

  for (; n > 0; n--) {
    if (wait(0) < 0) {
      printf("wait stopped early\n");
      exit(-1);
    }
  }

  if (wait(0) != -1) {
    printf("check many\n");
    printf("wait got too many\n");
    exit(-1);
  }
  printf("fork test OK\n");
}

void different_hello() {
  pid_t pid1 = fork();
  pid_t pid2 = fork();
  pid_t pid3 = fork();
  sleep(10);
  printf("Hello World from (%d, %d, %d)\n", pid1, pid2, pid3);
  while (wait(0) != -1)
    ;
  exit(0);
}

void ping_pong_thread() {
  printf("hello, thread?\n");
  int data = 0;
  int ret = fork();
  int i = 8;
  if (ret == 0) {
    data = 2;
    while (i != 0) {
      i--;
      printf("Child Process: Pong %d, %d;\n", data, i);
      sleep(10);
    }
    exit(10);
  } else if (ret != -1) {
    data = 1;
    while (i != 0) {
      i--;
      printf("Father Process: Ping %d, %d;\n", data, i);
      sleep(10);
    }
  }
  int32_t status;
  int pid2 = wait(&status);
  printf("child %d exited with code %d\n", pid2, status);
  exit(0);
}

int data = 0;

void producer(int arg, sem_t mutex, sem_t buffer) {
  int pid = getpid();
  for (int k = 1; k <= 8; ++k) {
    printf("pid %d, producer %d, produce, product %d\n", pid, arg, k);
    sleep(128);
    printf("pid %d, producer %d, try lock, product %d\n", pid, arg, k);
    sleep(128);
    sem_wait(mutex);
    sleep(128);
    printf("pid %d, producer %d, locked\n", pid, arg);
    sleep(128);
    sem_post(mutex);
    sleep(128);
    printf("pid %d, producer %d, unlock\n", pid, arg);
    sleep(128);
    sem_post(buffer);
  }
}

void consumer(int arg, sem_t mutex, sem_t buffer) {
  int pid = getpid();
  for (int k = 1; k <= 4; ++k) {
    printf("pid %d, consumer %d, try consume, product %d\n", pid, arg, k);
    sleep(128);
    sem_wait(buffer);
    sleep(128);
    printf("pid %d, consumer %d, try lock, product %d\n", pid, arg, k);
    sleep(128);
    sem_wait(mutex);
    sleep(128);
    printf("pid %d, consumer %d, locked\n", pid, arg);
    sleep(128);
    sem_post(mutex);
    sleep(128);
    printf("pid %d, consumer %d, unlock\n", pid, arg);
    sleep(128);
    printf("pid %d, consumer %d, consumed, product %d\n", pid, arg, k);
  }
}

void shell(){
  exec("app.elf", 0, NULL);
  exit(1);
}

void autoqed() {
  exec("autoqed.elf", 0, NULL);
  exit(1);
}

#define N 5
sem_t forks[N];
sem_t room;

void philosopher(int i) {
  int id = getpid() - 2;
  printf("Philosopher %d : think\n", id);
  sleep(128);
  sem_wait(room);
  sleep(128);
  sem_wait(forks[i]);
  sleep(128);
  sem_wait(forks[(i + 1) % N]);
  sleep(128);
  printf("Philosopher %d : eat\n", id);
  sleep(128);
  sem_post(forks[(i + 1) % N]);
  sleep(128);
  sem_post(forks[i]);
  sleep(128);
  sem_post(room);
  sleep(128);
}

void sem_test() {
  // // 测试scanf
  int dec = 0;
  int hex = 0;
  char str[6553];
  char cha = 0;
  int ret = 0;
  // while (1) {
  //   printf("Input:\" Test %%c Test %%s %%d %%x\"\n");
  //   ret = scanf(" Test %c Test %s %d %x", &cha, str, &dec, &hex);
  //   printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
  //   if (ret == 4)
  //     break;
  // }
  int i = 4;
  sem_t sem;
  printf("Father Process: Semaphore Initializing.\n");
  sem = sem_init(0);
  if (sem == -1) {
    printf("Father Process: Semaphore Initializing Failed.\n");
    exit(0);
  }

  ret = fork();
  if (ret == 0) {
    while (i != 0) {
      i--;
      printf("Child Process: Semaphore Waiting.\n");
      sem_wait(sem);
      printf("Child Process: In Critical Area.\n");
    }
    printf("Child Process: Semaphore Destroying.\n");
    sem_destroy(sem);
    exit(0);
  } else if (ret != -1) {
    while (i != 0) {
      i--;
      printf("Father Process: Sleeping.\n");
      sleep(28);
      printf("Father Process: Semaphore Posting.\n");
      sem_post(sem);
    }
    printf("Father Process: Semaphore Destroying.\n");
    sem_destroy(sem);
  }
  if (ret != 0) {
    while (wait(0) != -1)
      ;
  }
  // For lab4.3
  // TODO: You need to design and test the philosopher problem.
  // Note that you can create your own functions.
  // Requirements are demonstrated in the guide.

  //哲学家

  printf("philosopher\n");

  for (int i = 0; i < N; i++) {
    forks[i] = sem_init(1);
  }
  room = sem_init(4);
  for (int i = 0; i < 5; i++) {
    if ((ret = fork()) == 0) {
      philosopher(i);
      exit(0);
    }
  }
  if (ret != 0) {
    while (wait(0) != -1)
      ;
    for (int i = 0; i < 5; i++) {
      sem_destroy(forks[i]);
    }
  }
  int mutex = sem_init(1);
  int buffer = sem_init(0);
  printf("mutex : %d\nbuffer : %d\n", mutex, buffer);
  for (int i = 0; i < 6; ++i) {
    ret = fork();
    if (ret == 0) {
      if (i < 2)
        producer(i + 1, mutex, buffer);
      else
        consumer(i - 1, mutex, buffer);
      exit(0);
    }
  }

  if (ret != 0) {
    while (wait(0) != -1)
      ;
    sem_destroy(buffer);
    sem_destroy(mutex);
  }
  // 读者写者问题
}

int xqcl(void) {
  // forktest();
  // sem_test();
  dollar = sem_init(1);
  udp send;
  udp recv;
  sock_t port1 = socket_init();
  sock_t port2 = socket_init();;
  int ret = fork();
  if (ret == 0) { // child process
    recv.head.src = port1;
    recv.head.dst = port2;
    int re = 0;
    while (1) {
      sem_wait(dollar);
      re = socket_recv(port2, &recv);
      if (re == -1)
        goto turn;
      // printf("receive : %s\n", &recv.packet);
      if (!strcmp((char *)&(recv.packet), "exit")) {
        exit(0);
      } else if (!strcmp((char *)&(recv.packet), "forktest"))
        func_wrapper(forktest);
      else if (!strcmp((char *)&(recv.packet), "sem_test"))
        func_wrapper(sem_test);
      else if(!strcmp((char*)&(recv.packet), "shell"))
        func_wrapper(shell);
      else if(!strcmp((char*)&(recv.packet), "qed"))
        func_wrapper(autoqed);
      else
        printf("%s\n", &(recv.packet));
      turn:
      sem_post(dollar);
    }
  } else if (ret != -1) {
    memset(&send, 0, sizeof(send));
    send.head.src = port1;
    send.head.dst = port2;
    while (1) {
      sem_wait(dollar);
      printf("> ");
      static char buf[506];
      memset(buf, 0, sizeof(buf));
      scanf("%s", buf);
      socket_send((void *)buf, sizeof(buf), port1, port2);
      sem_post(dollar);
      if (!strcmp(buf, "exit")) {
        break;
      }
    }
  }
  while (wait(0) != -1)
    ;
  socket_destroy(port1);
  socket_destroy(port2);
  if(getpid()==1)trap();
  exit(0);
  return 0;
}
