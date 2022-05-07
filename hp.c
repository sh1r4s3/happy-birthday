#include <unistd.h>
#include <ncurses.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define REFRESH_USEC 6e4

enum WindowState {
    StateInit,
    StatePwd,
    StateMsg
};

static struct {
    WINDOW *hnd;
    int cx, cy;
    enum WindowState state;
    pthread_mutex_t stateMutex;
    char pwd[9];
    int pwdSz;
    pthread_mutex_t pwdMutex;
} window = {0};

static struct {
    const char *data;
    size_t dataSz;
    char *text;
    size_t pos;
} message = {0};

static void drawHeart();
static void drawHB();
static void drawPwd();
static void drawMessage(int x, int y);
static float incPhase(float phase, float dphi);
static void inputPwd(char ch);
static void *inputThread(void *);
static void logic();
static void decr();
static void drawDog();

int main() {
    const char data[] = {
        0x78, 0x56, 0x40, 0x45, 0x48, 0x19, 0x5b, 0x5c,
        0x72, 0x44, 0x5f, 0x54, 0x54, 0x48, 0x19, 0x4a,
        0x40, 0x6e, 0x43, 0x5f, 0x59, 0x5b, 0x54, 0x18,
        0x19, 0x7c, 0x20, 0x47, 0x5e, 0x43, 0x5d, 0x11,
        0x40, 0x56, 0x40, 0x20, 0x44, 0x5f, 0x51, 0x41,
        0x11, 0x40, 0x56, 0x40, 0x72, 0x10, 0x53, 0x42,
        0x50, 0x50, 0x54, 0x4a, 0x15, 0x63, 0x5f, 0x5a,
        0x55, 0x15, 0x45, 0x4b, 0x4c, 0x50, 0x21, 0x3a,
        0x7e, 0x10, 0x5e, 0x5f, 0x56, 0x4e, 0x19, 0x20,
        0x44, 0x5f, 0x51, 0x41, 0x11, 0x4d, 0x51, 0x50,
        0x20, 0x44, 0x5e, 0x5d, 0x50, 0x11, 0x50, 0x4a,
        0x15, 0x68, 0x51, 0x45, 0x43, 0x5d, 0x11, 0x57,
        0x56, 0x42, 0x2c, 0x10, 0x55, 0x45, 0x41, 0x11,
        0x70, 0x19, 0x5d, 0x6f, 0x40, 0x52, 0x10, 0x41,
        0x59, 0x58, 0x4d, 0x15, 0x61, 0x44, 0x17, 0x5c,
        0x50, 0x50, 0x4a, 0x4d, 0x15, 0x74, 0x5f, 0x53,
        0x51, 0x4c, 0x11, 0x40, 0x56, 0x40, 0x20, 0x47,
        0x5e, 0x5c, 0x59, 0x11, 0x5b, 0x5c, 0x15, 0x61,
        0x52, 0x5b, 0x55, 0x15, 0x45, 0x56, 0x19, 0x51,
        0x69, 0x43, 0x43, 0x42, 0x54, 0x52, 0x4d, 0x19,
        0x54, 0x20, 0x52, 0x5e, 0x44, 0x15, 0x57, 0x4b,
        0x56, 0x58, 0x20, 0x44, 0x5f, 0x5f, 0x46, 0x54,
        0x19, 0x58, 0x42, 0x66, 0x45, 0x5b, 0x10, 0x50,
        0x47, 0x5c, 0x57, 0x41, 0x73, 0x10, 0x56, 0x5e,
        0x51, 0x11, 0x51, 0x58, 0x43, 0x65, 0x10, 0x56,
        0x10, 0x59, 0x58, 0x4d, 0x4d, 0x59, 0x65, 0x10,
        0x51, 0x45, 0x5b, 0x1d, 0x19, 0x5f, 0x5c, 0x6e,
        0x54, 0x17, 0x51, 0x15, 0x5f, 0x5c, 0x4e, 0x15,
        0x70, 0x51, 0x5e, 0x42, 0x15, 0x5e, 0x5f, 0x19,
        0x46, 0x68, 0x5f, 0x52, 0x43, 0x19, 0x11, 0x5e,
        0x56, 0x15, 0x74, 0x5f, 0x17, 0x43, 0x5a, 0x5c,
        0x5c, 0x19, 0x53, 0x61, 0x5e, 0x54, 0x49, 0x15,
        0x5c, 0x4c, 0x4a, 0x50, 0x75, 0x5d, 0x17, 0x51,
        0x5b, 0x55, 0x19, 0x5c, 0x54, 0x74, 0x10, 0x56,
        0x10, 0x45, 0x5d, 0x58, 0x4d, 0x50, 0x20, 0x5f,
        0x51, 0x10, 0x57, 0x5e, 0x4b, 0x4a, 0x56, 0x68,
        0x10, 0x40, 0x59, 0x41, 0x59, 0x19, 0x4a, 0x5a,
        0x6d, 0x55, 0x17, 0x5c, 0x54, 0x43, 0x5d, 0x19,
        0x54, 0x6e, 0x54, 0x17, 0x40, 0x54, 0x5c, 0x49,
        0x4c, 0x46, 0x68, 0x5b, 0x5e, 0x11, 0x15, 0xb,
        0x10, 0x33, 0x3f, 0x4c, 0x5f, 0x41, 0x55, 0x15,
        0x48, 0x56, 0x4c, 0x19, 0x20, 0x7e, 0x5e, 0x5b,
        0x5c, 0x45, 0x58, 0x17, 0x3f
    };
    message.data = data;
    message.dataSz = sizeof(data);
    message.text = malloc(message.dataSz);

    window.hnd = initscr();
    getmaxyx(window.hnd, window.cy, window.cx);
    raw();
    noecho();

    srand(time(0));

    pthread_t pth;
    pthread_create(&pth, 0, inputThread, 0);
    pthread_mutex_init(&window.stateMutex, 0);
    pthread_mutex_init(&window.pwdMutex, 0);

    while (1) {
        clear();
        logic();
        refresh();
        usleep(REFRESH_USEC);
    }

    endwin();
    return 0;
}

void drawHeart() {
    float r = 3.;
    float x, y, t = 0;
    int xi = window.cx/2, yi = -(window.cy/2+r*5)*0.5;
    int xin, yin;

    for (float t = 0; t < 2*M_PI; t += M_PI/32) {
        x = r*16*pow(sin(t), 3);
        y = -r*(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))*0.5;
        xin = (int)x + window.cx/2;
        yin = (int)y + window.cy/2;
        if (xi != xin || yi != yin) {
            xi = xin;
            yi = yin;
            mvaddch(yi, xi, 'x');
        }
    }

}

void drawHB() {
    static float phase = 0.f;
    const char hp[] = "Happy Birthday!";
    const float scale = 2;
    const int xsparse = 4;
    const size_t hpsz = sizeof(hp);
    int wx, wy, x, y;
    float dphi = 0.;

    phase = incPhase(phase, 15.);

    wy = window.cy/2;
    wx = (window.cx - hpsz*xsparse)/2;
    x = wx;
    y = wy;

    for (const char *pch = hp; pch < hp+hpsz-1; pch++) {
        y = wy + (int)(sin((phase+dphi)/180.*M_PI)*scale);
        dphi += 60.;
        x += xsparse;
        mvaddch(y, x, *pch);
    }
}

float incPhase(float phase, float dphi) {
    phase += dphi;
    if (fabs(phase) >= 360.) {
        phase -= (int)(phase/360)*360.;
    }
    return phase;
}

void drawMessage(int x, int y) {
    int x0 = x;
    int npos = 0;
    if (message.pos < message.dataSz) ++message.pos;
    for (size_t nch = 0; nch < message.pos - 1; nch++) {
        char ch = message.text[nch];
        if ((ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            (ch == '.' || ch == ',' || ch == '\'') || ch == ':' || ch == ' ' || ch == '?' || ch == '!' || ch == ')') {

            if (npos > window.cx - 16) {
                x = x0;
                npos = 0;
                ++y;
            }
            mvaddch(y, x + npos++, message.text[nch]);
        } else if (ch == '\n') {
            x = x0;
            npos = 0;
            ++y;
        } else {
            if (npos > window.cx - 16) {
                x = x0;
                npos = 0;
                ++y;
            }
            mvaddch(y, x + npos++, '*');
        }
    }
}

void drawPwd() {
    const char pwdRnd[] = {'*', '#', '%'};
    const char pwd[] = "Input password: ";
    const size_t pwdSz = sizeof(pwd) - 1;
    const int x = window.cx/2 - pwdSz/2;
    const int y = 2;
    static int npwd = 0;
    static enum WindowState state = StateInit;

    if (npwd < pwdSz) ++npwd;
    else if (state != StatePwd) {
        state = StatePwd;
        pthread_mutex_lock(&window.stateMutex);
        window.state = StatePwd;
        pthread_mutex_unlock(&window.stateMutex);
    }

    for (int nch = 0; nch < npwd; ++nch) {
        mvaddch(y, x + nch, pwd[nch]);
    }

    pthread_mutex_lock(&window.pwdMutex);
    for (int nch = 0; nch < window.pwdSz; ++nch) {
        char rnd = pwdRnd[rand()%sizeof(pwdRnd)];
        mvaddch(y, x + npwd + nch, rnd);
    }
    pthread_mutex_unlock(&window.pwdMutex);
}

void inputPwd(char ch) {
    pthread_mutex_lock(&window.pwdMutex);
    if (window.pwdSz < sizeof(window.pwd) && ch >= '0' && ch <= '9') {
        window.pwd[window.pwdSz++] = ch;
    }
    if (ch == 0x7F && window.pwdSz > 0) {
        window.pwd[window.pwdSz--] = 0;
    }
    pthread_mutex_unlock(&window.pwdMutex);

    if (ch == '\n') {
        pthread_mutex_lock(&window.stateMutex);
        window.state = StateMsg;
        pthread_mutex_unlock(&window.stateMutex);
        decr();
    }
}

void *inputThread(void *a) {
    int ch = 0;
    while ((ch = getch()) != 'q') {
        pthread_mutex_lock(&window.stateMutex);
        const enum WindowState state = window.state;
        pthread_mutex_unlock(&window.stateMutex);

        switch (state) {
            case StatePwd:
                inputPwd((char)ch);
                break;
        }
    }
    pthread_mutex_destroy(&window.stateMutex);
    pthread_mutex_destroy(&window.pwdMutex);
    clear();
    endwin();
    exit(0);
    return 0;
}

void logic() {
    pthread_mutex_lock(&window.stateMutex);
    const enum WindowState state = window.state;
    pthread_mutex_unlock(&window.stateMutex);

    switch (state) {
        case StateInit:
        case StatePwd:
            drawHB();
            drawHeart();
            drawPwd();
            break;
        case StateMsg:
            drawMessage(4, 4);
            drawDog();
            break;
    }
}

void drawDog() {
    static int frame = 0;
    char al0[] = "                                                                    \n                                                                    \n                                                                    \n                        :N               .                          \n                       .M,O. MSUZBr   ,7jXB                         \n                       N: BM20 Li M8JqY,. ji                        \n                       B0r:         ;i JX M.                        \n                     7U:               i.B7                         \n                    qU:          iu.     ,M                         \n                   B7YB5  :q.    kB8,.    :B                        \n                  B7..  :LPGYU:    .:.     UF                       \n                 U1      r  :r              B.                      \n                 M.                         rE                      \n                 O:                          B.                     \n                 :M                          7q                     \n                  07                          B                     \n                  .B                          q7                    \n                   B.                         :G                    \n                   5v                          B.                   \n                   7P                          JBj.                 \n                   .B                            :USL .vj  .        \n                    B.                              iNB:LZLuZ.      \n                    0r                                iE    5u      \n                    7P                                 :B   :Mr     \n                    .B                                  FS   G7     \n                     B.                                 .B   B      \n                     Y5                              .,. Gr  B.     \n                      B                            ..,.. jY BU      \n                      j2                         ..,,... UBUY       \n                       Br                    ..,,:.,... .O          \n                     :uuB;        . . ....,.,.,......  .BX..,.:rBi  \n                   YNY   8U  ......,................ .JP. :rr7i:BB  \n             2X  i8v      iBB,    .............    :UXMU         :  \n             YBBMJ        2Y:5FY;:.           .:ruSSi  ,kF,         \n               ..       .B:    :7JF21uuYYYjUFU5J7,       ,11L:      \n                        B.           ..,.,..                .iBB    \n                    kBBB:                                     B7    \n                     .NU                                            \n                                                                    \n                                                                    \n";
    char al1[] = "                                                                    \n                                                                    \n                      .J2     v                                     \n                     YL,P2FP7B00:  iYLruS                           \n                    jk,EFi2, : .S2GL... M                           \n                   .ZF:          ., :Z.2:                           \n                  vv                  LB                            \n                 qLU7    .    iBu      UY                           \n                M7:BF  .Mi    ,MG:,     Gi                          \n               Zr    .7UB01r     .       B                          \n              ;k          .              v5                         \n              Ur                          M.                        \n              iS                          L2                        \n               M,                          M                        \n               ,O                          U7                       \n                0:                         :E                       \n                LY                          M                       \n                ,0                          25                      \n                 O                           U1L:                   \n                 E,                            :Lkr.:r              \n                 jL                               iBP,0J17          \n                 :k                                 j7   MU         \n                  M                                  Xr   jU        \n                  N:                                 .B.   M:       \n                  LU                                . q7  .0        \n                   M                              .,. rP   B,       \n                   S7                           ..... ,q :O,        \n                    M                        ..,..... i87Y   i      \n                    .q                  . ..,.,.....  BL::77rBB.    \n          YBBi .,::i,GB.   . . ........,..........  ,Z:      .2:    \n           :P0Lrri:.  :qr    ............ . . .   ,5B               \n                   .    Y8YL:      ... . .     .i1U;iLvr,.          \n                  7BBUr7rLYXqL7i:..     ..,:rLUuL.    .:777Bq       \n                   :Li       .:rvjjuJuYuJUL7i,             5B.      \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n";
    char al2[] = "                                                                    \n                   .                                                \n                  0Xu  r7 i5      :F5                               \n                 ur BPLB:Gk;Mv.:jL: 5,                              \n                 Z1vq:..    :7SX .r X:                              \n                Y1:              F7ju                               \n              i5.                  1S                               \n             JUrBu   ,.    0B2      U2                              \n            YZ.iu.  JB. ,   L7i.     Mi                             \n           :O ..  .SBBBM.    ...     .B                             \n           E:        ii               2Y                            \n           O.                          B                            \n           LF                          UJ                           \n            M.                         .M                           \n            i0                          G:                          \n             B                          rX                          \n             ki                         .B                          \n             LU                          8B.                        \n             .8                           7U17.  ..                 \n              M                              rU1E7E5rj7             \n              G:                               .Gr .. E7            \n              Lu                                 1j   iE            \n              :G                                  Oi   NX           \n               B                                  :B  .B            \n               Fv                               .  O:  G.           \n               ,O                              ,.. Uv JM            \n                Gi                           .,... 7S:E   58F       \n                 B                       ....,...  MBFiivULBN       \n                 .B                   ..,.,.,..   U5. .             \n                .7GM.    . ........,.,.,...... ,7Mk      :          \n              r5j.  Pv. . ........... . . .   ,XBY;vJYjYrBBY        \n          ,BBBj      EBBE,                .:7k1:         .ku        \n               .. iUj,  iY1Uj7r::,:,::r7jU5Jr                       \n               BBBr.        .:;77YvL77;i.                           \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n                                                                    \n";
    char al3[] = "                                                                    \n                                                                    \n                                                                    \n                    7uS  . 7P,     ::i                              \n                  :U,:G1BY7B:XL :Uv::Lq                             \n                  8Urk: .     ikk .v rY                             \n                 71:              ri:F                              \n               :U:                  2E                              \n              7F;B5   ,.    UBN..    rk                             \n             rN.:L. .JBi..   ;i:.     F7                            \n            ,Z     .Y55UU.             O                            \n            1i                         7U                           \n            P:                          M                           \n            iX                          72                          \n             0:                          G                          \n             .N                          5i                         \n              E.                         :N                         \n              j7                          G                         \n              i2                          2k.                       \n               Z                           :L27.                    \n               N.                             :Juj7Yi,.             \n               Fi                                Pu LrLX            \n               i5                                 7U   SF           \n               .E                                  S7   :0          \n                Z.                                 .M   ,N          \n                vY                               .. X;   Z          \n                .8                             .... vj  YZ          \n                 F7                          ...... iEiXj           \n                  O.                      ..,.....  S7,.            \n                   G               . ....,.,....   uM. ....::       \n                   ,B:  . ................... .  iNv .:i;iirBB      \n                  :5:2U,   ....... ... ... .   ,YZ57,       iU      \n                 2U   .72Er                 :rUui   iYji.           \n             kBB5:     :P7ruJJ7ri:,:,::rrLYuvi         i7JLr        \n              ir      kL     .::rr7r7;ri:.                :BS       \n                    :E,                                    2Y       \n                 .BB0                                               \n                                                                    \n                                                                    \n                                                                    \n";
    char al4[] = "                                                                    \n                                                                    \n                                                                    \n                       i                                            \n                      uXk. ,1::kr     :JM.                          \n                     ,N YBrBrLM:qE::Yj: iU                          \n                     rB7F;..     iLZ, U :F                          \n                    75i              iJr8                           \n                  .Xi                  :B                           \n                 :MiBB   ::    iBB..    .B                          \n                ,B:,7: ,;BU.:   :ri,.    r8                         \n                B.     jr iq:             G;                        \n               7S                         .M                        \n               Yu                          qr                       \n                B                          .B                       \n                J5                          E:                      \n                 B.                         rX                      \n                 2Y                          B                      \n                 :N                          0i                     \n                  M                          iB7                    \n                  M.                          .L5Ui   i             \n                  27                             .L1BULMvu1.        \n                  rS                                Lk  . iO        \n                   M                                 ,M    B:       \n                   M.                                 rG   rB       \n                   1v                                  B:  E:       \n                   ,O                               .. YP  YU       \n                    M,                             ,.. :G :B:       \n                    iE                          ..,... .BrB,        \n                     qY                      ..,.,.... UZ:          \n                      M;              ......,.,...... 7O            \n                       Z2  . ..........,.,........   2F             \n                       kqXi    .......... .....   .LFqL.            \n                      O:  iBB:.               .:7FU01  L1r          \n                     E7   .Z,iuU1YLri::::i77uU1Jr.  i0.  iBu        \n                    jU   .B      ,:rr777rri:.        .qL  B8        \n                 .BBB    M,                            iB:          \n                 .ri  iBB7                             iBN          \n                      :q7                                           \n                                                                    \n";
    int x = 2, y = window.cy/4;
    ++frame;
    char *al;
    size_t alSz;
    if (frame >= 24) frame = 0;
    if (frame < 4) {
        al = al0;
        alSz = sizeof(al0);
    } else if (frame < 8) {
        al = al1;
        alSz = sizeof(al1);
    } else if (frame < 16) {
        al = al2;
        alSz = sizeof(al2);
    } else if (frame < 20) {
        al = al3;
        alSz = sizeof(al3);
    }
    else if (frame < 24) {
        al = al4;
        alSz = sizeof(al4);
    }

    for (int n = 0; n < alSz - 1; ++n) {
        if (al[n] == '\n') { ++y; x = 2; }
        else               mvaddch(y, x++, al[n]);
    }
}

void decr() {
    int npwd = 0;
    for (int nch = 0; nch < message.dataSz; ++nch) {
        message.text[nch] = message.data[nch]^window.pwd[npwd];
        if (npwd < window.pwdSz) ++npwd;
        else                     npwd = 0;
    }
}
