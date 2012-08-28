#include <stdio.h>
#define A 100

enum test{
    a = A*1,
    b = A*2,
};

void main()
{
    printf("a = %d, b = %d \n",a,b);
}
