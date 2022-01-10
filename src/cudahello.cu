
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef CMAKE_CUDA_COMPILER
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#endif
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#define _(str) gettext(str)

int main(void)
{
#ifdef CMAKE_CUDA_COMPILER
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
        printf("Error : %s \n%s\n", cudaGetErrorName(t),
               cudaGetErrorString(t));
    }
#endif
    return EXIT_SUCCESS;
}
