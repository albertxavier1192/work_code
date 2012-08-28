//#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <term.h>
#include <termios.h>
#include <unistd.h>

int getch(void)
{
    int ch;
    struct termios buf;
    struct termios save;

    tcgetattr(0, &save);
    buf = save;
    buf.c_lflag &= ~(ICANON|ECHO);
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &buf);
    ch = getchar();
    tcsetattr(0, TCSAFLUSH, &save);
    return ch;
}


int main()
{
    int ch;
    for(;;) {
	ch = getch();
	if(ch == 0xE0 || ch == 0){
	    ch = getch();
	    printf("Ȯ�� Ű �Է�, �ڵ� = %d\n",ch);
	}else{
	    printf("�Ϲ� ���� �Է�, ���� = %c, �ڵ� = %d\n",ch, ch);
	    if(ch == 'q') exit(0);
	}
	//break;
    }
}


