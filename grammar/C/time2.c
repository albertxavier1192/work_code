#include <stdio.h>
#include <sys/time.h>
#include <time.h>

int main(void)
{
    struct timeval ts;
    gettimeofday(&ts, NULL);
    //time_t now;
    //now = localtime(ts); // 지역표준시로 변환한다 (대한민국은 KST)
    /*
    //ts = gmtime(&now);  // 국제표준시 GMT로 변환한다
    */
    char buf[100];
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", ts);
    printf("%ld -> %s\n", now, buf);

    return 0;
}
