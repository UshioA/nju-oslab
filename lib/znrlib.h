#ifndef __ZNR__
#define __ZNR__

#include "types.h"
int strcmp(char *a, char *b);

int strncmp(char *a, char *b, unsigned n);

unsigned strlen(char *s);

int strStr(char *haystack, char *needle);

char *strcpy(char *des, const char *str);


char *strncpy(char *dest, const char *src, unsigned cnt);

void *memcpy(void *str1, const void *str2, int size);

void *memset(void *ptr, int val, unsigned num);

char *strcat(char *dest, const char *src);

char *strncat(char *strDes, const char *strSrc, unsigned cnt);

char *strtok(char *str, const char *s);

char *strchr(const char *str, int value);

char *strrchr(char *str, int character);

size_t strcspn(const char *str1, const char *str2);

size_t strspn(const char *str1, const char *str2);

void *memmove(void *dest, void *src, size_t len);

void itoa(int number, char *str, int radix);
int atoi(char *a, int base, int *len);
#endif