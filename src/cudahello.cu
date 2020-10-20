
#if defined(__GNUC__)
#if __GNUC__ < 8
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
  int device = 0;
  cudaError_t t = cudaSuccess;
  printf(_("Hello World!\n"));
  t = cudaGetDeviceCount(&device);
  if (!t)
  {
    printf("%d, %d\n", t, device);
  }
  else
  {
    printf("Error : %s \n%s\n", cudaGetErrorName(t), cudaGetErrorString(t));
  }
  return EXIT_SUCCESS;
}
#endif
#endif
