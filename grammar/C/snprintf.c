#include <stdio.h>
#include <string.h>

int main()
{
    char buff[100];
    char buff2[100] = {"asdjflasjdlf"};
    int i =0;

    i += snprintf(buff, 100, "test");
    snprintf(buff+i, 100-i, "abcd");
    buff[0] = 0;
    strcpy(buff2, buff);
    printf("a%sa\n", buff2);

    return 1;
}


