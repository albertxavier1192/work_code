#include <stdio.h>
#include <sys/time.h>
#include <time.h>

int main(void)
{
    struct timeval ts;
    gettimeofday(&ts, NULL);
    //time_t now;
    //now = localtime(ts); // ����ǥ�ؽ÷� ��ȯ�Ѵ� (���ѹα��� KST)
    /*
    //ts = gmtime(&now);  // ����ǥ�ؽ� GMT�� ��ȯ�Ѵ�
    */
    char buf[100];
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
    printf("%ld -> %s\n", now, buf);

    return 0;
}
