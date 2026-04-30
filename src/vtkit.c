#include "vtkit/vtkit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vtk_buffer_cell {
    char ch;
    unsigned char fg;
    unsigned char bg;
} vtk_buffer_cell;

static vtk_buffer_cell *g_buffer_front = NULL;
static vtk_buffer_cell *g_buffer_back = NULL;
static int g_buffer_width = 0;
static int g_buffer_height = 0;

static int g_vtk_width  = 0;
static int g_vtk_height = 0;

static int vtk_clamp_color(int n) {
    if (n < 0) {
        return 0;
    }
    if (n > 7) {
        return 7;
    }
    return n;
}

static size_t vtk_buffer_index(int x, int y) {
    return (size_t) y * (size_t) g_buffer_width + (size_t) x;
}

int vtk_buffer_init(int width, int height) {
    size_t count;

    if (width <= 0 || height <= 0) {
        return -1;
    }

    vtk_buffer_free();

    count = (size_t) width * (size_t) height;

    g_buffer_front = (vtk_buffer_cell *) malloc(count * sizeof(vtk_buffer_cell));
    g_buffer_back = (vtk_buffer_cell *) malloc(count * sizeof(vtk_buffer_cell));
    if (g_buffer_front == NULL || g_buffer_back == NULL) {
        vtk_buffer_free();
        return -1;
    }

    g_buffer_width = width;
    g_buffer_height = height;

    vtk_buffer_clear(' ', VTK_WHITE, VTK_BLACK);
    memset(g_buffer_front, 0, count * sizeof(vtk_buffer_cell));
    return 0;
}

void vtk_buffer_free(void) {
    free(g_buffer_front);
    free(g_buffer_back);
    g_buffer_front = NULL;
    g_buffer_back = NULL;
    g_buffer_width = 0;
    g_buffer_height = 0;
}

void vtk_buffer_clear(char ch, int fg, int bg) {
    size_t i;
    size_t count;
    vtk_buffer_cell fill;

    if (g_buffer_back == NULL) {
        return;
    }

    count = (size_t) g_buffer_width * (size_t) g_buffer_height;
    fill.ch = ch;
    fill.fg = (unsigned char) vtk_clamp_color(fg);
    fill.bg = (unsigned char) vtk_clamp_color(bg);

    for (i = 0; i < count; ++i) {
        g_buffer_back[i] = fill;
    }
}

void vtk_buffer_put(int x, int y, char ch, int fg, int bg) {
    size_t idx;

    if (g_buffer_back == NULL) {
        return;
    }
    if (x < 0 || x >= g_buffer_width || y < 0 || y >= g_buffer_height) {
        return;
    }

    idx = vtk_buffer_index(x, y);
    g_buffer_back[idx].ch = ch;
    g_buffer_back[idx].fg = (unsigned char) vtk_clamp_color(fg);
    g_buffer_back[idx].bg = (unsigned char) vtk_clamp_color(bg);
}

void vtk_buffer_present(void) {
    int x;
    int y;
    int last_fg = -1;
    int last_bg = -1;

    if (g_buffer_front == NULL || g_buffer_back == NULL) {
        return;
    }

    for (y = 0; y < g_buffer_height; ++y) {
        for (x = 0; x < g_buffer_width; ++x) {
            size_t idx = vtk_buffer_index(x, y);
            vtk_buffer_cell next = g_buffer_back[idx];
            vtk_buffer_cell prev = g_buffer_front[idx];

            if (next.ch == prev.ch && next.fg == prev.fg && next.bg == prev.bg) {
                continue;
            }

            if ((int) next.fg != last_fg) {
                vtk_color(next.fg);
                last_fg = next.fg;
            }
            if ((int) next.bg != last_bg) {
                vtk_bg(next.bg);
                last_bg = next.bg;
            }

            vtk_goto(x, y);
            putchar(next.ch);
            g_buffer_front[idx] = next;
        }
    }
}

void vtk_buffer_hline(int x, int y, int len, char ch, int fg, int bg) {
    int i;

    for (i = 0; i < len; ++i) {
        vtk_buffer_put(x + i, y, ch, fg, bg);
    }
}

void vtk_buffer_vline(int x, int y, int len, char ch, int fg, int bg) {
    int i;

    for (i = 0; i < len; ++i) {
        vtk_buffer_put(x, y + i, ch, fg, bg);
    }
}

void vtk_buffer_text(int x, int y, const char *text, int fg, int bg) {
    int i;

    if (text == NULL) {
        return;
    }
    for (i = 0; text[i] != '\0'; ++i) {
        vtk_buffer_put(x + i, y, text[i], fg, bg);
    }
}

void vtk_buffer_rect(int x, int y, int w, int h, char ch, int fg, int bg) {
    int row;

    for (row = 0; row < h; ++row) {
        vtk_buffer_hline(x, y + row, w, ch, fg, bg);
    }
}

void vtk_buffer_line(int x, int y, int dx, int dy, char ch, int fg, int bg) {
    int x0 = x;
    int y0 = y;
    int x1 = x + dx;
    int y1 = y + dy;
    int adx = dx < 0 ? -dx : dx;
    int ady = dy < 0 ? -dy : dy;
    int sx = dx < 0 ? -1 : 1;
    int sy = dy < 0 ? -1 : 1;
    int err;

    if (adx == 0 && ady == 0) {
        vtk_buffer_put(x0, y0, ch, fg, bg);
        return;
    }

    if (adx >= ady) {
        err = adx / 2;
        while (x0 != x1) {
            vtk_buffer_put(x0, y0, ch, fg, bg);
            err -= ady;
            if (err < 0) {
                y0 += sy;
                err += adx;
            }
            x0 += sx;
        }
    } else {
        err = ady / 2;
        while (y0 != y1) {
            vtk_buffer_put(x0, y0, ch, fg, bg);
            err -= adx;
            if (err < 0) {
                x0 += sx;
                err += ady;
            }
            y0 += sy;
        }
    }
    vtk_buffer_put(x1, y1, ch, fg, bg);
}

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <io.h>
#include <windows.h>

static DWORD g_old_console_mode_in;
static DWORD g_old_console_mode_out;
static int g_has_raw_mode = 0;
static int g_has_vt_mode = 0;

static void vtk_enable_vt_processing(void) {
    HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    if (h_out == INVALID_HANDLE_VALUE) {
        return;
    }
    if (!GetConsoleMode(h_out, &mode)) {
        return;
    }

    g_old_console_mode_out = mode;

    if (SetConsoleMode(h_out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        g_has_vt_mode = 1;
    }
}

void vtk_clear(void) {
    vtk_enable_vt_processing();
    fputs("\x1b[2J\x1b[H", stdout);
    fflush(stdout);
}

void vtk_goto(int x, int y) {
    vtk_enable_vt_processing();
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    fprintf(stdout, "\x1b[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void vtk_color(int n) {
    vtk_enable_vt_processing();
    n = vtk_clamp_color(n);
    fprintf(stdout, "\x1b[%dm", 30 + n);
    fflush(stdout);
}

void vtk_bg(int n) {
    vtk_enable_vt_processing();
    n = vtk_clamp_color(n);
    fprintf(stdout, "\x1b[%dm", 40 + n);
    fflush(stdout);
}

void vtk_reset(void) {
    vtk_enable_vt_processing();
    fputs("\x1b[0m", stdout);
    fflush(stdout);
}

void vtk_hide_cursor(void) {
    vtk_enable_vt_processing();
    fputs("\x1b[?25l", stdout);
    fflush(stdout);
}

void vtk_show_cursor(void) {
    vtk_enable_vt_processing();
    fputs("\x1b[?25h", stdout);
    fflush(stdout);
}

int vtk_raw_mode(void) {
    HANDLE h_in;
    DWORD mode;

    h_in = GetStdHandle(STD_INPUT_HANDLE);
    if (h_in == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!GetConsoleMode(h_in, &mode)) {
        return -1;
    }

    g_old_console_mode_in = mode;

    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    mode |= ENABLE_PROCESSED_INPUT;

    if (!SetConsoleMode(h_in, mode)) {
        return -1;
    }

    g_has_raw_mode = 1;
    vtk_enable_vt_processing();
    return 0;
}

int vtk_restore_mode(void) {
    HANDLE h_in;

    if (!g_has_raw_mode) {
        return 0;
    }

    h_in = GetStdHandle(STD_INPUT_HANDLE);
    if (h_in == INVALID_HANDLE_VALUE) {
        return -1;
    }

    if (!SetConsoleMode(h_in, g_old_console_mode_in)) {
        return -1;
    }

    if (g_has_vt_mode) {
        HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
        if (h_out != INVALID_HANDLE_VALUE) {
            SetConsoleMode(h_out, g_old_console_mode_out);
        }
        g_has_vt_mode = 0;
    }

    g_has_raw_mode = 0;
    return 0;
}

int vtk_getch(void) {
    if (_kbhit()) {
        return _getch();
    }
    return -1;
}

int vtk_get_width(void) {
    if (g_vtk_width == 0) {
        g_vtk_width = 80;
    }
    return g_vtk_width;
}

int vtk_get_height(void) {
    if (g_vtk_height == 0) {
        g_vtk_height = 24;
    }
    return g_vtk_height;
}

void vtk_watch_resize(int enable) {
    (void) enable;
}

int vtk_handle_resize(void) {
    return 0;
}

#else

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_old_termios;
static int g_old_flags = 0;
static int g_has_raw_mode = 0;

void vtk_clear(void) {
    fputs("\x1b[2J\x1b[H", stdout);
    fflush(stdout);
}

void vtk_goto(int x, int y) {
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    fprintf(stdout, "\x1b[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

void vtk_color(int n) {
    n = vtk_clamp_color(n);
    fprintf(stdout, "\x1b[%dm", 30 + n);
    fflush(stdout);
}

void vtk_bg(int n) {
    n = vtk_clamp_color(n);
    fprintf(stdout, "\x1b[%dm", 40 + n);
    fflush(stdout);
}

void vtk_reset(void) {
    fputs("\x1b[0m", stdout);
    fflush(stdout);
}

void vtk_hide_cursor(void) {
    fputs("\x1b[?25l", stdout);
    fflush(stdout);
}

void vtk_show_cursor(void) {
    fputs("\x1b[?25h", stdout);
    fflush(stdout);
}

int vtk_raw_mode(void) {
    struct termios raw;
    int flags;

    if (g_has_raw_mode) {
        return 0;
    }

    if (tcgetattr(STDIN_FILENO, &g_old_termios) != 0) {
        return -1;
    }

    raw = g_old_termios;
    raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        return -1;
    }

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags < 0) {
        (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_old_termios);
        return -1;
    }

    g_old_flags = flags;
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) != 0) {
        (void) tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_old_termios);
        return -1;
    }

    g_has_raw_mode = 1;
    return 0;
}

int vtk_restore_mode(void) {
    if (!g_has_raw_mode) {
        return 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_old_termios) != 0) {
        return -1;
    }

    if (fcntl(STDIN_FILENO, F_SETFL, g_old_flags) != 0) {
        return -1;
    }

    g_has_raw_mode = 0;
    return 0;
}

int vtk_getch(void) {
    fd_set read_fds;
    struct timeval timeout;
    unsigned char c;
    int ready;
    ssize_t nread;

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    ready = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
    if (ready < 0) {
        if (errno == EINTR) {
            return -1;
        }
        return -1;
    }
    if (ready == 0 || !FD_ISSET(STDIN_FILENO, &read_fds)) {
        return -1;
    }

    nread = read(STDIN_FILENO, &c, 1);
    if (nread == 1) {
        return (int) c;
    }

    return -1;
}

static void vtk_query_size(void) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) { g_vtk_width  = (int) ws.ws_col; }
        if (ws.ws_row > 0) { g_vtk_height = (int) ws.ws_row; }
    }
    if (g_vtk_width  == 0) { g_vtk_width  = 80; }
    if (g_vtk_height == 0) { g_vtk_height = 24; }
}

int vtk_get_width(void) {
    if (g_vtk_width == 0) {
        vtk_query_size();
    }
    return g_vtk_width;
}

int vtk_get_height(void) {
    if (g_vtk_height == 0) {
        vtk_query_size();
    }
    return g_vtk_height;
}

static volatile sig_atomic_t g_resized = 0;

static void vtk_sigwinch_handler(int sig) {
    (void) sig;
    g_resized = 1;
}

void vtk_watch_resize(int enable) {
    struct sigaction sa;
    sa.sa_handler = enable ? vtk_sigwinch_handler : SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);
}

int vtk_handle_resize(void) {
    if (!g_resized) {
        return 0;
    }
    g_resized = 0;

    /* Re-query and cache the new terminal size. */
    g_vtk_width  = 0;
    g_vtk_height = 0;
    vtk_query_size();

    if (g_vtk_width < 2 || g_vtk_height < 2) {
        return 0;
    }

    vtk_buffer_free();
    if (vtk_buffer_init(g_vtk_width, g_vtk_height) != 0) {
        return 0;
    }

    vtk_clear();
    return 1;
}

#endif

void vtk_begin(void) {
    vtk_clear();
    vtk_hide_cursor();
}

void vtk_end(void) {
    vtk_show_cursor();
    vtk_reset();
}
