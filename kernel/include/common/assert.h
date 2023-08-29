#ifndef __ASSERT_H__
#define __ASSERT_H__

int abort(const char *, int);

//* 随处可见的assert, 断言条件为真, 若为假则蓝屏退出
#define assert(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      Say("\e[31mAssertion `%s' Failed\e[0m", #cond);                          \
      asm volatile("cli");                                                     \
      while (1)                                                                \
        asm volatile("hlt");                                                   \
    }                                                                          \
  } while (0)

//* 比较厉害的assert, 能辱骂写代码的人.
#define Assert(cond, fmt, ...)                                                 \
  do {                                                                         \
    if (!(cond)) {                                                             \
      Say("\e[31mAssertion `%s' Failed\e[0m -> " fmt, #cond, ##__VA_ARGS__);   \
      asm volatile("cli");                                                     \
      while (1)                                                                \
        asm volatile("hlt");                                                   \
    }                                                                          \
  } while (0)

#endif
