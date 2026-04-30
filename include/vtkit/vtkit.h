#ifndef VTKIT_VTKIT_H
#define VTKIT_VTKIT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(VTK_BUILD)
    #define VTK_API __declspec(dllexport)
  #else
    #define VTK_API __declspec(dllimport)
  #endif
#else
  #if defined(__GNUC__) && __GNUC__ >= 4
    #define VTK_API __attribute__((visibility("default")))
  #else
    #define VTK_API
  #endif
#endif

typedef enum vtk_color_code {
    VTK_BLACK = 0,
    VTK_RED = 1,
    VTK_GREEN = 2,
    VTK_YELLOW = 3,
    VTK_BLUE = 4,
    VTK_MAGENTA = 5,
    VTK_CYAN = 6,
    VTK_WHITE = 7,
    VTK_BRIGHT_BLACK = 8,
    VTK_BRIGHT_RED = 9,
    VTK_BRIGHT_GREEN = 10,
    VTK_BRIGHT_YELLOW = 11,
    VTK_BRIGHT_BLUE = 12,
    VTK_BRIGHT_MAGENTA = 13,
    VTK_BRIGHT_CYAN = 14,
    VTK_BRIGHT_WHITE = 15
} vtk_color_code;

/* 256-color helpers: RGB cube (16-231) and grayscale (232-255). */
/* Each of r, g, b must be in 0..5; level must be in 0..23.      */
static inline int vtk_rgb(int r, int g, int b) {
    return 16 + (r * 36) + (g * 6) + b;
}

static inline int vtk_gray(int level) {
    return 232 + (level < 24 ? level : 23);
}

VTK_API void vtk_clear(void);
VTK_API void vtk_goto(int x, int y);
VTK_API void vtk_color(int n);
VTK_API void vtk_bg(int n);
VTK_API void vtk_reset(void);
VTK_API void vtk_hide_cursor(void);
VTK_API void vtk_show_cursor(void);
VTK_API int vtk_raw_mode(void);
VTK_API int vtk_restore_mode(void);
VTK_API int vtk_getch(void);

/* Terminal size. */
VTK_API int vtk_get_width(void);
VTK_API int vtk_get_height(void);

/* Resize handling. */
VTK_API void vtk_watch_resize(int enable);
VTK_API int  vtk_handle_resize(void);

/* Session helpers. */
VTK_API void vtk_begin(void);
VTK_API void vtk_end(void);

/* Generic frame buffer API (diff-rendered on present). */
VTK_API int vtk_buffer_init(int width, int height);
VTK_API void vtk_buffer_free(void);
VTK_API void vtk_buffer_clear(char ch, int fg, int bg);
VTK_API void vtk_buffer_put(int x, int y, char ch, int fg, int bg);
VTK_API void vtk_buffer_present(void);

/* Buffer draw primitives. */
VTK_API void vtk_buffer_hline(int x, int y, int len, char ch, int fg, int bg);
VTK_API void vtk_buffer_vline(int x, int y, int len, char ch, int fg, int bg);
VTK_API void vtk_buffer_text(int x, int y, const char *text, int fg, int bg);
VTK_API void vtk_buffer_rect(int x, int y, int w, int h, char ch, int fg, int bg);
VTK_API void vtk_buffer_line(int x, int y, int dx, int dy, char ch, int fg, int bg);

#ifdef __cplusplus
}
#endif

#endif
