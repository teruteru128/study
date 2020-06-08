
#include <stdio.h>
#include <stdlib.h>
static double ymax = 0;

void plot_start(long x, long y)
{
  printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
  printf("<svg xmlns=\"http://www.w3.org/2000/svg\" ");
  printf("version=\"1.1\" width=\"%ld\" height=\"%ld\">\n", (long)x, (long)y);
  printf("<path d=\"");
  ymax = y;
}

void plot_end(int plot_close)
{
  if (plot_close)
    printf("Z");
  printf("\" fill=\"none\" stroke=\"black\" />\n");
  printf("</svg>\n");
}

void move(double x, double y)
{
  printf("M %lf %lf \n", x, ymax - y);
}

void move_rel(double dx, double dy)
{
  printf("m %.25lf %.25lf \n", dx, -dy);
}

void draw(double x, double y)
{
  printf("L %lf %lf \n", x, ymax - y);
}

void draw_rel(double dx, double dy)
{
  printf("l %lf %lf\n", dx, -dy);
}
