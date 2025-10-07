#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "../dpp.hpp"

using namespace dpp::literals;

#define WIDTH      80
#define HEIGHT     24
#define FPS        60
#define DURATION   4.0          /* seconds until implosion */

static char front[HEIGHT][WIDTH];
static char back[HEIGHT][WIDTH];

using D = dpp::d64;

static D t_start;

static D now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void clear(char buf[HEIGHT][WIDTH])
{
    memset(buf, ' ', sizeof(char) * WIDTH * HEIGHT);
}

static void show(char buf[HEIGHT][WIDTH])
{
    printf("\033[H");          /* home cursor */
    for (int y = 0; y < HEIGHT; ++y) {
        fwrite(buf[y], 1, WIDTH, stdout);
        putchar('\n');
    }
    fflush(stdout);
}

static void point(int x, int y, char c, char buf[HEIGHT][WIDTH])
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        buf[y][x] = c;
}

static void circle(D cx, D cy, D r, char c, char buf[HEIGHT][WIDTH])
{
    const int steps = (int)(r * 8);
    for (int i = 0; i < steps; ++i) {
        D ang = 2_d64 * M_PI * i / steps;
        int x = (int)(cx + r * cos(ang));
        int y = (int)(cy + r * sin(ang) * 0.5); /* aspect-ratio fix */
        point(x, y, c, buf);
    }
}

static void ring(D cx, D cy, D r, char c, char buf[HEIGHT][WIDTH])
{
    const int steps = (int)(r * 8);
    for (int i = 0; i < steps; ++i) {
        D ang = 2 * M_PI * i / steps;
        int x0 = (int)(cx + (r - 0.5_d64) * cos(ang));
        int y0 = (int)(cy + (r - 0.5_d64) * sin(ang) * 0.5_d64);
        int x1 = (int)(cx + (r + 0.5_d64) * cos(ang));
        int y1 = (int)(cy + (r + 0.5_d64) * sin(ang) * 0.5_d64);
        point(x0, y0, c, buf);
        point(x1, y1, c, buf);
    }
}

static void shockwave(D t)
{
    clear(back);

    D center_x = WIDTH  / 2_d64;
    D center_y = HEIGHT / 2_d64;

    /* Expansion phase 0..1 */
    D phase = fmin(t / DURATION, 1_d64);

    /* Ease-out elastic for dramatic bounce */
    D elastic = phase * phase * (3_d64 - 2_d64 * phase);
    D radius  = 1_d64 + elastic * 30_d64;

    /* Primary ring */
    ring(center_x, center_y, radius, '#', back);

    /* Secondary ripples */
    for (int i = 1; i <= 3; ++i) {
        D lag = phase - i * 0.15_d64;
        if (lag > 0) {
            D r = 1.0_d64 + fmin(lag, 1.0_d64) * 30.0_d64;
            ring(center_x, center_y, r, ".-*"[i - 1], back);
        }
    }

    /* Implosion spark shower at the end */
    if (phase >= 0.95_d64) {
        D f = (phase - 0.95_d64) * 20.0_d64; /* 0..1 over last 5 % */
        int sparks = (int)(50 * f);
        for (int i = 0; i < sparks; ++i) {
            D ang = 2_d64 * M_PI * rand() / RAND_MAX;
            D r   = f * (rand() % 15);
            int x = (int)(center_x + r * cos(ang));
            int y = (int)(center_y + r * sin(ang) * 0.5_d64);
            point(x, y, '+', back);
        }
    }

    show(back);
}

int main(void)
{
    printf("\033[?25l"); /* hide cursor */
    printf("\033[2J");   /* clear screen  */
    t_start = now();
    srand((unsigned)time(NULL));

    while (1) {
        D t = now() - t_start;
        if (t > DURATION + 0.5_d64) break;
        shockwave(t);
        usleep(1000000 / FPS);
    }

    printf("\033[?25h"); /* restore cursor */
    return 0;
}
