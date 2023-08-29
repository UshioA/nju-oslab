#include "lib.h"
#include "znrlib.h"
// #define C114514(x) if((x)==num)printf("%s = %d\n", (#x), (num));
// #define C11451(x) C114514(x##4);C114514(x+4);C114514(x-4);C114514(x*4);C114514(x/4);
// #define C1145(x) C11451(x##1);C11451(x+1);C11451(x-1);C11451(x*1);C11451(x/1);
// #define C114(x) C1145(x##5);C1145(x+5);C1145(x-5);C1145(x*5);C1145(x/5);
// #define C11(x) C114(x##4);C114(x+4);C114(x-4);C114(x*4);C114(x/4);
// #define C1(x) C11(x##1);C11(x+1);C11(x-1);C11(x*1);C11(x/1);

#define C123(x) if((x)==num)printf("%s = %d\n", (#x), (num));
#define C12(x) C123(x##3);C123(x+3);C123(x-3);C123(x*3);C123(x/3);
#define C1(x) C12(x##2);C12(x+2);C12(x-2);C12(x*2);C12(x/2);

int main(void){
	int num = 0;
  printf("enter one number: ");
  scanf("%d", &num);
	C1(1);
	C1(-1);
  exit(0);
}



