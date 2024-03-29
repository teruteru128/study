
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <svgplot.h>

/*
http://utthi-fumi.hatenablog.com/entry/2019/04/09/135235
*/
void dragon(int i, double dx, double dy, int sign)
{
    if (i == 0)
    {
        draw_rel(dx, dy);
    }
    else
    {
        dragon(i - 1, (dx - sign * dy) / 2, (dy + sign * dx) / 2, 1);
        dragon(i - 1, (dx + sign * dy) / 2, (dy - sign * dx) / 2, -1);
    }
}

int main(int argc, char *argv[])
{
    int order = 64;
    plot_start(40000, 25000);
    move(100, 100);
    dragon(order, 1L << 32, 0, 1);
    plot_end(0);
    return EXIT_SUCCESS;
}
