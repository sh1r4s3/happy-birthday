#include <unistd.h>
#include <ncurses.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

void drawSinStr(WINDOW *wnd, float phase) {
    const char hp[] = "Happy Birthday!";
    const float scale = 2;
    const size_t hpsz = sizeof(hp);
    int wx, wy, x, y;
    float dphi = 0.;

    getmaxyx(wnd, wy, wx);
    wy /= 2;
    wx = (wx - hpsz*3)/2;
    x = wx;
    y = wy;

    clear();
    for (const char *pch = hp; pch < hp+hpsz-1; pch++) {
        y = wy + (int)(sin((phase+dphi)/180.*M_PI)*scale);
        dphi += 60.;
        x += 3;
        mvaddch(y, x, *pch);
    }
    refresh();
}

float incPhase(float phase, float dphi) {
    phase += dphi;
    if (fabs(phase) >= 360.) {
        phase -= (int)(phase/360)*360.;
    }
    return phase;
}

void *quit(void *) {
    getch();
    endwin();
    exit(0);
    return 0;
}

int main() {
    float phase = 0.;

    WINDOW *wnd = initscr();
    raw();

    pthread_t pth;
    pthread_create(&pth, 0, quit, 0);

    while (1) {
        phase = incPhase(phase, 15.);
        drawSinStr(wnd, phase);
        usleep(3e4);
    }

    endwin();
    return 0;
}
