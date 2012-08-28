#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

int main(){
    initsrc();

    move(1,1);
    printw("%s","hello");
    refresh();
    sleep(2);

    endwin();
    return 0;
}

