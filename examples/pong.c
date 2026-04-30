#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "vtkit/vtkit.h"
#include "demo_utils.h"

enum {
    PADDLE_H = 4,
    FRAME_USEC_DEFAULT = 50000,
    FRAME_USEC_MIN = 8000,
    FRAME_USEC_MAX = 200000,
    FRAME_USEC_STEP = 5000,
    KEY_REPEAT_INITIAL_USEC_DEFAULT = 0,
    KEY_REPEAT_INTERVAL_USEC_DEFAULT = 30000,
    KEY_REPEAT_INITIAL_MIN_USEC = 0,
    KEY_REPEAT_INTERVAL_MIN_USEC = 5000,
    KEY_REPEAT_MAX_USEC = 300000,
    KEY_HOLD_RELEASE_TIMEOUT_USEC_DEFAULT = 400000,
    KEY_HOLD_RELEASE_TIMEOUT_USEC_MIN = 20000,
    KEY_HOLD_RELEASE_TIMEOUT_USEC_MAX = 1000000,
    SCREEN_W_MAX = 120,
    SCREEN_H_MAX = 40
};

/* Runtime geometry — set from terminal size at startup. */
static int g_screen_w;
static int g_screen_h;
static int g_total_h;
static int g_left_x;
static int g_right_x;

static int g_frame_usec = FRAME_USEC_DEFAULT;
static int g_key_repeat_initial_usec = KEY_REPEAT_INITIAL_USEC_DEFAULT;
static int g_key_repeat_interval_usec = KEY_REPEAT_INTERVAL_USEC_DEFAULT;
static int g_key_hold_release_timeout_usec = KEY_HOLD_RELEASE_TIMEOUT_USEC_DEFAULT;

static int clamp(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

void pong_set_frame_delay_usec(int usec) {
    g_frame_usec = clamp(usec, FRAME_USEC_MIN, FRAME_USEC_MAX);
}

void pong_set_key_repeat_usec(int initial_usec, int interval_usec) {
    g_key_repeat_initial_usec = clamp(initial_usec, KEY_REPEAT_INITIAL_MIN_USEC, KEY_REPEAT_MAX_USEC);
    g_key_repeat_interval_usec = clamp(interval_usec, KEY_REPEAT_INTERVAL_MIN_USEC, KEY_REPEAT_MAX_USEC);
}

void pong_set_key_hold_release_timeout_usec(int timeout_usec) {
    g_key_hold_release_timeout_usec = clamp(
        timeout_usec,
        KEY_HOLD_RELEASE_TIMEOUT_USEC_MIN,
        KEY_HOLD_RELEASE_TIMEOUT_USEC_MAX
    );
}

static void draw_border(void) {
    vtk_buffer_hline(0, 0, g_screen_w, '-', VTK_WHITE, VTK_BLACK);
    vtk_buffer_hline(0, g_screen_h - 1, g_screen_w, '-', VTK_WHITE, VTK_BLACK);
    vtk_buffer_vline(0, 1, g_screen_h - 2, '|', VTK_WHITE, VTK_BLACK);
    vtk_buffer_vline(g_screen_w - 1, 1, g_screen_h - 2, '|', VTK_WHITE, VTK_BLACK);
}

static void draw_paddle(int x, int y_top, int color) {
    vtk_buffer_vline(x, y_top, PADDLE_H, '#', color, VTK_BLACK);
}

static void draw_ball(int x, int y) {
    vtk_buffer_put(x, y, 'O', VTK_YELLOW, VTK_BLACK);
}

static void draw_status_line(int left_score, int right_score) {
    char line[256];

    if (snprintf(
            line,
            sizeof(line),
            "W/S move  +/- speed  Q quit  |  delay=%dms  |  %d:%d",
            g_frame_usec / 1000,
            left_score,
            right_score
        ) < 0) {
        return;
    }

    vtk_buffer_hline(0, g_screen_h, g_screen_w, ' ', VTK_GREEN, VTK_BLACK);
    vtk_buffer_text(0, g_screen_h, line, VTK_GREEN, VTK_BLACK);
}

int main(int argc, char **argv) {
    int left_y;
    int right_y;
    int ball_x;
    int ball_y;
    int vx;
    int vy;
    int left_score = 0;
    int right_score = 0;
    int ch;
    int delay_arg;
    int repeat_initial_arg;
    int repeat_interval_arg;
    int release_timeout_arg;
    int left_move_dir = 0;
    long long left_key_last_seen_usec = 0;
    long long left_next_repeat_usec = 0;
    long long now;
    int running = 1;

    if (argc > 1) {
        delay_arg = atoi(argv[1]);
        if (delay_arg > 0) {
            pong_set_frame_delay_usec(delay_arg * 1000);
        }
    }
    if (argc > 2) {
        repeat_initial_arg = atoi(argv[2]);
        if (repeat_initial_arg > 0) {
            pong_set_key_repeat_usec(repeat_initial_arg * 1000, g_key_repeat_interval_usec);
        }
    }
    if (argc > 3) {
        repeat_interval_arg = atoi(argv[3]);
        if (repeat_interval_arg > 0) {
            pong_set_key_repeat_usec(g_key_repeat_initial_usec, repeat_interval_arg * 1000);
        }
    }
    if (argc > 4) {
        release_timeout_arg = atoi(argv[4]);
        if (release_timeout_arg > 0) {
            pong_set_key_hold_release_timeout_usec(release_timeout_arg * 1000);
        }
    }

    srand((unsigned int) time(NULL));

    g_screen_w = clamp(vtk_get_width(),  20, SCREEN_W_MAX);
    g_screen_h = clamp(vtk_get_height() - 1, 10, SCREEN_H_MAX);
    g_total_h  = g_screen_h + 1;
    g_left_x   = 2;
    g_right_x  = g_screen_w - 3;

    if (vtk_raw_mode() != 0) {
        fprintf(stderr, "Failed to enable raw mode\n");
        return EXIT_FAILURE;
    }

    if (vtk_buffer_init(g_screen_w, g_total_h) != 0) {
        vtk_restore_mode();
        fprintf(stderr, "Failed to initialize terminal buffer\n");
        return EXIT_FAILURE;
    }

    left_y = (g_screen_h - PADDLE_H) / 2;
    right_y = (g_screen_h - PADDLE_H) / 2;
    ball_x = g_screen_w / 2;
    ball_y = g_screen_h / 2;
    vx = -1;
    vy = 1;

    vtk_begin();

    while (running) {
        now = demo_now_usec();

        while ((ch = vtk_getch()) != -1) {
            if (ch == 'q' || ch == 'Q') {
                running = 0;
                break;
            }
            if (ch == 'w' || ch == 'W') {
                left_move_dir = -1;
                left_y += left_move_dir;
                left_key_last_seen_usec = now;
                left_next_repeat_usec = now + ((g_key_repeat_initial_usec > 0) ? g_key_repeat_initial_usec : g_key_repeat_interval_usec);
            } else if (ch == 's' || ch == 'S') {
                left_move_dir = 1;
                left_y += left_move_dir;
                left_key_last_seen_usec = now;
                left_next_repeat_usec = now + ((g_key_repeat_initial_usec > 0) ? g_key_repeat_initial_usec : g_key_repeat_interval_usec);
            } else if (ch == '+' || ch == '=') {
                pong_set_frame_delay_usec(g_frame_usec - FRAME_USEC_STEP);
            } else if (ch == '-' || ch == '_') {
                pong_set_frame_delay_usec(g_frame_usec + FRAME_USEC_STEP);
            }
        }

        now = demo_now_usec();
        if (left_move_dir != 0) {
            if ((now - left_key_last_seen_usec) > g_key_hold_release_timeout_usec) {
                left_move_dir = 0;
            } else {
                while (now >= left_next_repeat_usec) {
                    left_y += left_move_dir;
                    left_next_repeat_usec += g_key_repeat_interval_usec;
                }
            }
        }

        left_y = clamp(left_y, 1, g_screen_h - 1 - PADDLE_H);

        if (ball_y < right_y + (PADDLE_H / 2)) {
            right_y -= 1;
        } else if (ball_y > right_y + (PADDLE_H / 2)) {
            right_y += 1;
        }
        right_y = clamp(right_y, 1, g_screen_h - 1 - PADDLE_H);

        ball_x += vx;
        ball_y += vy;

        if (ball_y <= 1) {
            ball_y = 1;
            vy = 1;
        } else if (ball_y >= g_screen_h - 2) {
            ball_y = g_screen_h - 2;
            vy = -1;
        }

        if (ball_x == g_left_x + 1 && ball_y >= left_y && ball_y < left_y + PADDLE_H) {
            ball_x = g_left_x + 1;
            vx = 1;
        }
        if (ball_x == g_right_x - 1 && ball_y >= right_y && ball_y < right_y + PADDLE_H) {
            ball_x = g_right_x - 1;
            vx = -1;
        }

        if (ball_x <= 1) {
            right_score += 1;
            ball_x = g_screen_w / 2;
            ball_y = g_screen_h / 2;
            vx = 1;
            vy = (rand() % 2 == 0) ? -1 : 1;
        } else if (ball_x >= g_screen_w - 2) {
            left_score += 1;
            ball_x = g_screen_w / 2;
            ball_y = g_screen_h / 2;
            vx = -1;
            vy = (rand() % 2 == 0) ? -1 : 1;
        }

        vtk_buffer_clear(' ', VTK_WHITE, VTK_BLACK);
        draw_border();
        draw_paddle(g_left_x, left_y, VTK_CYAN);
        draw_paddle(g_right_x, right_y, VTK_MAGENTA);
        draw_ball(ball_x, ball_y);

        draw_status_line(left_score, right_score);

        vtk_buffer_present();

        fflush(stdout);
        usleep((useconds_t) g_frame_usec);
    }

    vtk_goto(0, g_screen_h);
    vtk_end();
    vtk_restore_mode();
    vtk_buffer_free();
    vtk_clear();

    return EXIT_SUCCESS;
}
