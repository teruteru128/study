
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "gettext.h"
#define _(str)   gettext(str)
#define N_(str)  gettext_noop(str)
#include <locale.h>

#define N 200
#define DEVICE 0

__global__ void sum_of_array(float *arr1, float *arr2, float *arr3, int size)
{
  // https://gist.github.com/onionmk2/854c333829f047a5e86cfab5a0ccae3a
  // x y zの3次元を1次元に直す。zは xとyの重みが、yはxの重みが、xは1の重みがあると考える。
  int block_id = blockIdx.z * (gridDim.x * gridDim.y) + blockIdx.y * (gridDim.x) + blockIdx.x;
  int threadId = block_id * (blockDim.x * blockDim.y * blockDim.z) + (threadIdx.z * (blockDim.x * blockDim.y)) + (threadIdx.y * blockDim.x) + threadIdx.x;
  arr3[threadId] = arr1[threadId] + arr2[threadId];
}

void initialize_array(float *arr, size_t size)
{
  for (size_t i = 0; i < size; i++)
  {
    arr[i] = (float)rand();
  }
}

/* https://nonbiri-tereka.hatenablog.com/entry/2017/04/11/081601 */
int main(void)
{
  float *arr1, *arr2, *arr3;
  float *d_arr1 = NULL, *d_arr2 = NULL, *d_arr3 = NULL;
  size_t n_byte = N * sizeof(float);
  int device = 0;
  cudaError_t error = cudaSuccess;
  error = cudaGetDeviceCount(&device);
  if (error)
  {
    printf(_("Error : %s\n%s\n"), cudaGetErrorName(error), cudaGetErrorString(error));
    return EXIT_FAILURE;
  }
  else
  {
    printf(ngettext("Success : one cuda device\n", "Success : %d cuda devices\n", device), device);
  }
  cudaDeviceProp prop;
  for (int dev = 0; dev < device; dev++)
  {
    cudaGetDeviceProperties(&prop, dev);
    printf("%s compute capability ", prop.name);
    printf("%d.%d\n", prop.major, prop.minor);
  }
  device = DEVICE;
  cudaGetDeviceProperties(&prop, device);
  printf("Using %s\n\n", prop.name);
  cudaSetDevice(device);

  arr1 = (float *)malloc(n_byte);
  arr2 = (float *)malloc(n_byte);
  arr3 = (float *)malloc(n_byte);

  initialize_array(arr1, n_byte);
  initialize_array(arr2, n_byte);
  initialize_array(arr3, n_byte);

  fputs("start cudaMalloc\n", stderr);
  cudaMalloc((void **)&d_arr1, n_byte);
  fputs("d_arr1\n", stderr);
  cudaMalloc((void **)&d_arr2, n_byte);
  fputs("d_arr2\n", stderr);
  cudaMalloc((void **)&d_arr3, n_byte);
  fputs("finish cudaMalloc\n", stderr);

  fputs("start cudaMemcpy\n", stderr);
  cudaMemcpy(d_arr1, arr1, n_byte, cudaMemcpyHostToDevice);
  cudaMemcpy(d_arr2, arr2, n_byte, cudaMemcpyHostToDevice);
  cudaMemcpy(d_arr3, arr3, n_byte, cudaMemcpyHostToDevice);
  fputs("finish cudaMemcpy\n", stderr);

  fputs("start kernel function\n", stderr);
  sum_of_array<<<(N + 255) / 256, 256>>>(d_arr1, d_arr2, d_arr3, n_byte);
  fputs("finish kernel function\n", stderr);
  cudaMemcpy(arr3, d_arr3, n_byte, cudaMemcpyDeviceToHost);
  size_t i = 0;
  for (; i < 100; i++)
  {
    fprintf(stderr, "%f\n", arr3[i]);
  }
  return EXIT_SUCCESS;
}
