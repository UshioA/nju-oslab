#ifndef __CONFIG__
#define __CONFIG__
#define DEBUG
// #define GRAPHIC_VGA
#define TEXT_VGA

#ifdef TEXT_VGA
#ifdef GRAPHIC_VGA
#error 怎么定义了两个VGA捏
#endif
#endif

#ifdef GRAPHIC_VGA
#ifdef TEXT_VGA
#error 怎么定义了两个VGA捏
#endif
#endif

#endif