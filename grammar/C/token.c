#include <stdio.h>
#include <string.h>
#define DELIM "/"
#define DELIM2 "_"

int main(void)
{
    char buf[] = "My_name_is_ubuntu/asdf";
    char tmp[4][10] = {};
    char *token;
    char *token2;
    char *ptr[2];
    int i = 0;

    token = strtok_r(buf, DELIM, &ptr[0]);
    while( token )
    {
	    printf(" : %s\n",token);
	token2 = strtok_r(tmp[i], DELIM2, &ptr[1]);
	while( token2 )
	{
	    printf(" : %s\n",token2);
	    token2 = strtok_r(NULL, DELIM, &ptr[1]);
	}
	token = strtok_r(NULL, DELIM, &ptr[0]);
	i++;
    }

    for(i=0; i<4; i++)

    return 0;
}
