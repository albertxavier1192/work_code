#include <stdio.h>

void test(char temp[][4])
{
    temp[1][2] =2; 
}

void main()
{
    int t=5;
    /*int ar[t]={1,2,3,4,5}; */
    int ar[t];
    int i;

    for(i=0; i<5; i++)
    {
	printf("%d\n",1[ar]);
    }

    char temp[3][4];
    test(temp);
    printf("%d\n",temp[1][2]);
}
