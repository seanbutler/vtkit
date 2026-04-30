#include <stdio.h>
#include <stdlib.h>

#include "vtkit/vtkit.h"

int main(void) {
    int ch;

    if (vtk_raw_mode() != 0) {
        fprintf(stderr, "Failed to enable raw mode\n");
        return EXIT_FAILURE;
    }

    vtk_clear();
    vtk_hide_cursor();
    vtk_color(VTK_YELLOW);
    vtk_bg(VTK_BLACK);
    vtk_goto(2, 1);
    printf("term demo: press q to quit");
    vtk_reset();
    fflush(stdout);

    for (;;) {
        ch = vtk_getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        }
    }

    vtk_show_cursor();
    vtk_reset();
    vtk_restore_mode();
    vtk_clear();
    return EXIT_SUCCESS;
}
