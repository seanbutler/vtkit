#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vtkit/vtkit.h"
#include "demo_utils.h"

enum {
    BOXES_PER_KEY = 4
};

static int g_screen_w;
static int g_screen_h;

static const char GLYPHS[] = "#*+=-~.:oO@";

static int rnd(int n) {
    return rand() % n;
}

static void draw_batch(void) {
    int i;
    int x;
    int y;
    int w;
    int h;
    int fg;
    int bg;
    char ch;

    for (i = 0; i < BOXES_PER_KEY; ++i) {
        fg = rnd(8);
        bg = rnd(8);
        ch = GLYPHS[rnd((int)(sizeof(GLYPHS) - 1))];

        /* pick a random origin and a size that fits in the buffer */
        x = rnd(g_screen_w - 1);
        y = rnd(g_screen_h - 1);
        w = rnd(g_screen_w - x) + 1;
        h = rnd(g_screen_h - y) + 1;

        vtk_buffer_rect(x, y, w, h, ch, fg, bg);
    }
}

int main(void) {
    int ch;

    g_screen_w = vtk_get_width();
    g_screen_h = vtk_get_height();

    if (vtk_raw_mode() != 0) {
        fprintf(stderr, "Failed to enable raw mode\n");
        return EXIT_FAILURE;
    }

    if (vtk_buffer_init(g_screen_w, g_screen_h) != 0) {
        vtk_restore_mode();
        fprintf(stderr, "Failed to initialize terminal buffer\n");
        return EXIT_FAILURE;
    }

    srand((unsigned int) time(NULL));

    vtk_watch_resize(1);
    vtk_begin();

    draw_batch();
    vtk_buffer_present();
    fflush(stdout);

    for (;;) {
        ch = demo_wait_key();
        if (ch == 'q' || ch == 'Q') {
            break;
        }
        if (vtk_handle_resize()) {
            g_screen_w = vtk_get_width();
            g_screen_h = vtk_get_height();
        }
        draw_batch();
        vtk_buffer_present();
        fflush(stdout);
    }

    vtk_end();
    vtk_restore_mode();
    vtk_buffer_free();
    vtk_clear();

    return EXIT_SUCCESS;
}
