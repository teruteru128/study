
#include "config.h"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#define _(str) gettext(str)

int main(void)
{
  printf(_("Hello World!\n"));
  return EXIT_SUCCESS;
}
