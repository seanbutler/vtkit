# TODO

## API

- [x] Move generic double-buffering from pong demo into the library
- [x] Refactor pong to use library buffer calls (`init`, `clear`, `put`, `present`, `free`)
- [x] Reuse shared internal color clamping logic in the library

- [x] Add lightweight buffer draw primitives in the library
- [x] Add `term_buffer_text(x, y, text, fg, bg)`
- [x] Add `term_buffer_hline(x, y, len, ch, fg, bg)`
- [x] Add `term_buffer_vline(x, y, len, ch, fg, bg)`
- [x] Add `term_buffer_rect(x, y, w, h, ch, fg, bg)`
- [x] Add `term_buffer_line(x, y, dx, dy, ch, fg, bg)`

- [x] Build a simple line/color demo: draw random lines with random colors/attributes, wait for keypress, draw more, repeat until quit

- [x] Refactor pong to use draw primitives for border, paddles, and status rendering
- [x] Add or expand README docs with examples for the new draw primitives
- [x] Add a small API demo snippet showing primitive drawing plus `term_buffer_present()`

## Helpers

- [x] Optionally add session helpers to reduce app boilerplate
- [x] Add `term_begin()` (clear plus hide cursor)
- [x] Add `term_end()` (show cursor plus reset)

## Utils

- [x] Keep timing and repeat logic out of the public API by extracting demo-only utilities into a shared examples helper module
- [x] Add a second demo using shared example utilities to validate reuse
- [x] Build and smoke-test all demos after each refactor milestone


## Terminal Size

- [x] Add `term_get_width()` returning the current terminal column count
- [x] Add `term_get_height()` returning the current terminal row count
- [x] Use `ioctl(TIOCGWINSZ)` on POSIX and `GetConsoleScreenBufferInfo` on Windows
- [x] Return sensible fallbacks (e.g. 80×24) if the query fails
- [x] Update README to document the two new functions
- [x] Update the demos so they work on any size screen

- [x] Update the API so it exposes the resize API cleanly and refactor the demos to access the functions from there
- [x] Maintain terminal width/height state inside the library; `term_handle_resize` updates it internally so callers can drop the `int *w, int *h` out-params and just read back via `term_get_width()`/`term_get_height()`


## Tests

- [ ] Create `tests/` folder with a minimal test harness (no external deps)
- [ ] Test group: buffer lifecycle (`init`, `free`, double-init, zero/negative size)
- [ ] Test group: `term_buffer_put` and `term_buffer_clear` (bounds, clamped colors, fill correctness)
- [ ] Test group: draw primitives — `hline`, `vline`, `text`, `rect`, `line` (length, position, content)
- [ ] Test group: `term_buffer_line` edge cases (zero delta, axis-aligned, diagonal, clipped)
- [ ] Wire tests into CMakeLists.txt with `enable_testing()` and `add_test()`
- [ ] Add test run step to README build instructions

## Rename to vtkit

- [x] Rename all `term_` function and type prefixes to `vtk_`
- [x] Rename header `include/term/term.h` → `include/vtkit/vtkit.h`
- [x] Rename source `src/term.c` → `src/vtkit.c`
- [x] Update `CMakeLists.txt`: project name, library targets, include paths, install rules
- [x] Update `cmake/termConfig.cmake.in` → `vtkitConfig.cmake.in`
- [x] Rename all example `#include` paths and update `demo_utils.h`
- [x] Update `README.md` to reflect new name and API prefix
- [x] Rename the project directory from `term/` to `vtkit/`
- [ ] Push the project to https://github.com/seanbutler/vtkit
