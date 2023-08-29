#include "znrlib.h"
int strcmp(char *a, char *b) {
  while (*a && *b) {
    if (*a - *b)
      return *a - *b;
    ++a, ++b;
  }
  return *a || *b;
}

void *memmove(void *dest, void *src, size_t len) {
  char *d = dest;
  char *s = src;
  if (d < s)
    while (len--)
      *d++ = *s++;
  else {
    char *lasts = s + (len - 1);
    char *lastd = d + (len - 1);
    while (len--)
      *lastd-- = *lasts--;
  }
  return dest;
}

int strncmp(char *a, char *b, unsigned n) {
  while (*a && *b && n--) {
    if (*a - *b)
      return *a - *b;
    ++a, ++b;
  }
  return *a || *b;
}

unsigned strlen(char *s) {
  int cnt = 0;
  while (*s) {
    ++cnt;
    ++s;
  }
  return cnt;
}

int strStr(char *haystack, char *needle) {
  int m = strlen(needle);
  if (m == 0)
    return 0;
  int n = strlen(haystack);
  int next[m];
  next[0] = 0;
  for (int i = 1, j = 0; i < m; i++) {
    while (j > 0 && needle[i] != needle[j])
      j = next[j - 1];
    if (needle[i] == needle[j])
      j++;
    next[i] = j;
  }
  for (int i = 0, j = 0; i < n; i++) {
    while (j > 0 && haystack[i] != needle[j])
      j = next[j - 1];
    if (haystack[i] == needle[j])
      j++;
    if (j == m)
      return i - m + 1;
  }
  return -1;
}

char *strcpy(char *des, const char *str) {
  char *tmp = des; 
  while (*str != '\0') {
    *des = *str;
    ++des;
    ++str;
  }
  *des = '\0'; 
  return tmp;
}



char *strncpy(char *dest, const char *src, unsigned cnt) {

  char *retAddr = dest;

  while (cnt && (*dest++ = *src++))
    cnt--;

  if (cnt) {
    while (--cnt) {
      *dest++ = '\0';
    }
  }
  return retAddr;
}

void *memcpy(void *str1, const void *str2, int size) {
  for (int i = 0; i < size; i++) {
    *((char *)str1 + i) = *((char *)str2 + i);
  }
  return str1;
}

void *memset(void *ptr, int val, unsigned num) {
  unsigned char ch = val; 
  for (unsigned i = 0; i < num; i++) {
    *((char *)ptr + i) = ch;
  }
  return ptr;
}

char *strcat(char *dest, const char *src) {
  if (!dest || !src) {
    return NULL;
  }
  int len = strlen(dest);
  int i = 0;
  while (src[i] != '\0') {
    dest[len + i] = src[i];
    ++i;
  }
  dest[len + i] = '\0';
  return dest;
}

char *strncat(char *strDes, const char *strSrc, unsigned cnt) {
  char *retAddr = strDes;

  while (*strDes)
    strDes++;

  while (cnt--)
    if (!(*strDes++ = *strSrc++)) 
      return retAddr;

  *strDes = '\0';
  return retAddr;
}

int atoi(char *a, int base, int *len) {
  int i = 0;
  char *origin = a;
  if (base == 16) {
    while (*a != 0) {
      int tmp = *a;
      if (tmp <= '9') {
        tmp = tmp - '0';
      } else {
        tmp = tmp - 'a' + 10;
        if (tmp < 10 || tmp > 15)
          break;
      }
      i = (i << 4) + tmp;
      ++a;
    }
  } else {
    while (*a) {
      int tmp = *a;
      tmp = tmp - '0';
      if (tmp < 0 || tmp > 9) {
        break;
      }
      i = i * base + tmp;
      ++a;
    }
  }
  if (len)
    *len = a - origin;
  return i;
}

void itoa(int number, char *str, int radix) {
  int k = 0;
  while (number != 0) {
    str[k++] = '0' + number % radix;
    number /= radix;
  }
  int left = 0, right = k - 1;
  
  while (left < right) {
    char temp = str[left];
    str[left] = str[right];
    str[right] = temp;
    left++;
    right--;
  }
  str[k] = '\0';
}

char *strchr(const char *str, int value) {
  if (str == NULL)
    return NULL;
  char c = (char)value;
  while (*str != c && *str++)
    ;
  if (*--str == '\0')
    return NULL;
  return (char *)++str;
}

char *strrchr(char *str, int character) {
  if (str == NULL)
    return NULL;
  char c = (char)character;
  register int i = 0;
  while (*(str + i++))
    ;
  while (--i && *(str + i) != c)
    ;
  if (i == 0) { 
    if (*str == c)
      return str;
    else
      return NULL;
  }
  return str + i;
}

size_t strcspn(const char *str1, const char *str2) {
  const char *s = str1, *key;
  while (*s) {
    key = str2;
    while (*key && *s != *key)
      key++;
    if (*key)
      return (s - str1);
    s++;
  }
  return (s - str1);
}

size_t strspn(const char *str1, const char *str2) {
  if (str1 == NULL || str2 == NULL)
    return 0;
  register int j;
  int i = 0;
  int n = 0;
  while (*(str2 + n++))
    ;
  n--; 
  while (*(str1 + i)) {
    j = 0;
    while (*(str2 + j++) != *(str1 + i)) {
      if (j == n)
        return i;
    }
    i++;
  }
  return i;
}

char *strtok(char *str, const char *delimiters) {
  if (delimiters == NULL)
    return NULL;
  static char *s_mem = NULL;
  if (str == NULL && s_mem == NULL)
    return NULL;

  char *s;
  if (str != NULL)
    s = str;
  else
    s = s_mem; 
               
  char const *delim;

  
  int stat = 1;
  while (stat) {
    delim = delimiters; 
    while (*delim && *s != *delim) {
      delim++;
    }
    
    if (*delim) { 
      s++;
    } else
      stat = 0;
  }
  s_mem = s; 

  
  while (*s) {
    delim = delimiters;
    while (*delim && *s != *delim) {
      delim++;
    }
    
    
    
    if (*delim) {
      *s = '\0';
      char *t = s_mem;
      s_mem = s + 1;
      return t;
    }
    s++;
  }
  
  char *t = s_mem;
  s_mem = NULL;
  return t;
}