#include <stdio.h>

void main()
{
    char array[10] = {"test"};
    char *array1 = "test11";

    int b=1;
    int *a = &b;

    if(*a)
    printf("%d\n",*a);

    int a1 = 15;
    int b2 = 2;
    if(!(a1 & b2))
	printf("ok\n");
}
