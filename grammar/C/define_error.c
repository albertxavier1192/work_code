#include <stdio.h>
#define I 3

void main()
{
#error testaaa
    int i=9;
#if (I>2)
    printf("test %s\n",__DATE__);
#endif
}
