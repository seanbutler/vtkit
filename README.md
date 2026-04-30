# vtkit

Small C terminal control library that builds as a shared library for reuse in other projects.

## API

- `vtk_clear()` clear screen, cursor to 0,0
- `vtk_goto(x, y)` move cursor to column `x`, row `y`
- `vtk_color(n)` set foreground color (`0..7`)
- `vtk_bg(n)` set background color (`0..7`)
- `vtk_reset()` reset fg/bg to terminal defaults
- `vtk_hide_cursor()` hide cursor
- `vtk_show_cursor()` show cursor
- `vtk_raw_mode()` enable non-blocking single-char input
- `vtk_restore_mode()` restore normal terminal input mode
- `vtk_getch()` returns keycode `int` or `-1` if no key pressed
- `vtk_get_width()` current terminal column count (fallback 80)
- `vtk_get_height()` current terminal row count (fallback 24)
- `vtk_buffer_init(width, height)` initialize a generic back/front buffer pair
- `vtk_buffer_free()` release buffer memory
- `vtk_buffer_clear(ch, fg, bg)` fill back buffer
- `vtk_buffer_put(x, y, ch, fg, bg)` write one cell in back buffer
- `vtk_buffer_present()` diff-render back buffer to terminal and update front buffer
- `vtk_buffer_hline(x, y, len, ch, fg, bg)` draw a horizontal run of cells
- `vtk_buffer_vline(x, y, len, ch, fg, bg)` draw a vertical run of cells
- `vtk_buffer_text(x, y, text, fg, bg)` write a string left-to-right
- `vtk_buffer_rect(x, y, w, h, ch, fg, bg)` fill a rectangle
- `vtk_buffer_line(x, y, dx, dy, ch, fg, bg)` draw a Bresenham line from `(x,y)` to `(x+dx, y+dy)`

Session helpers:

- `vtk_begin()` clear screen and hide cursor
- `vtk_end()` show cursor and reset attributes

Color constants are provided in `enum vtk_color_code`:

- `VTK_BLACK`, `VTK_RED`, `VTK_GREEN`, `VTK_YELLOW`
- `VTK_BLUE`, `VTK_MAGENTA`, `VTK_CYAN`, `VTK_WHITE`

## Build

```bash
cmake -S . -B build
cmake --build build
```

This produces:

- Linux/macOS: `libvtkit.so` or `libvtkit.dylib`
- Windows: `term.dll` (plus import library)

By default, a static library is also built:

- Linux/macOS: `libvtk_static.a`
- Windows: `vtk_static.lib`

Disable static library build if desired:

```bash
cmake -S . -B build -DVTK_BUILD_STATIC=OFF
```

## Install

```bash
cmake --install build --prefix ./install
```

Installs headers and CMake package files so other projects can use `find_package(term CONFIG REQUIRED)`.

## Use from another CMake project

```cmake
find_package(term CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE term::term)
```

To link the static variant (when built):

```cmake
target_link_libraries(my_app PRIVATE term::vtk_static)
```

## Demos

Build examples (default enabled):

```bash
cmake -S . -B build
cmake --build build
```

Run simple API demo:

```bash
./build/vtk_demo
```

Run animated Pong demo:

```bash
./build/vtk_pong
```

Run lines/color demo (any key draws a new batch, `Q` to quit):

```bash
./build/vtk_lines
```

Start Pong with a custom delay (milliseconds per frame):

```bash
./build/vtk_pong 70
```

Set frame delay plus key repeat timing (frame_ms initial_repeat_ms repeat_interval_ms):

```bash
./build/vtk_pong 70 80 25
```

Use `0` for `initial_repeat_ms` to remove extra first-hold delay (first repeat uses the normal repeat interval):

```bash
./build/vtk_pong 70 0 25
```

Optional 4th argument sets hold release timeout in ms (how long custom repeat continues after last key event):

```bash
./build/vtk_pong 70 0 25 400
```

Recommended to remove first-hold gap on many terminals: `initial_repeat_ms=0` and `hold_release_timeout_ms=350..500`.

Controls for Pong demo:

- `W` / `S`: move left paddle up/down
- `+` / `-`: adjust speed while running
- `Q`: quit

Pong uses an internal key repeat controller for the player paddle, so initial hold delay and repeat interval can be tuned independently of your OS keyboard repeat settings.

The Pong renderer now uses the library's generic diff-rendered buffer and only redraws changed cells each frame to reduce flicker.
