
#include <stdio.h>
#include <stdlib.h>
static double ymax = 0;

void plot_start(int x, int y)
{
  printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
  printf("<svg xmlns=\"http://www.w3.org/2000/svg\" ");
  printf("version=\"1.1\" width=\"%d\" height=\"%d\">\n", x, y);
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
  printf("M %lf %lf ", x, ymax - y);
}

void move_rel(double dx, double dy)
{
  printf("m %lf %lf ", dx, -dy);
}

void draw(double x, double y)
{
  printf("L %lf %lf ", x, ymax - y);
}

void draw_rel(double dx, double dy)
{
  printf("l %lf %lf ", dx, -dy);
}
