
#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 220
#endif
#ifdef CL_TARGET_OPENCL_VERSION
#include <CL/cl.h>
#include <CL/opencl.h>
#endif
#include <stdio.h>
#include <string.h>

const char *clGetErrorString(cl_int err);

/**
 * oepncl sample
 * kernel.cl を読み込ませる
 */
int clsample(const char *kernel_file)
{
    const int W = 625;
    const int size = W * W;
    int *h_res = (int *)calloc(size, sizeof(int));
    cl_int ret;

	// 1. プラットフォーム・デバイス取得
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    // 2. コンテキスト・コマンドキュー作成
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, NULL);

    // 3. バッファ作成
    cl_mem d_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * size, NULL, NULL);
    cl_mem found_seeds = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(long) * 1000, NULL, NULL);
    cl_mem found_count = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, NULL);

    // 4. カーネルの読み込み・コンパイル
    char source[BUFSIZ];
    FILE *in = fopen(kernel_file, "r");
    if(!in)
    {
        perror("kernel.cl not found");
        return 1;
    }
    fread(source, 1, BUFSIZ, in);
    fclose(in);
    size_t sourceLength = strlen(source);
    const char *input[] = {source, NULL};
    cl_program program = clCreateProgramWithSource(context, 1, input, &sourceLength, &ret);
    if(ret){
        fprintf(stderr, "clCreateProgramWithSource: %s\n", clGetErrorString(ret));
        return 1;
    }
    ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if(ret){
        fprintf(stderr, "clBuildProgram: %s\n", clGetErrorString(ret));
		char log[10000];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
		printf("Build Log:\n%s\n", log);
        return 1;
    }
    cl_kernel kernel = clCreateKernel(program, "search_perfect_seeds", &ret);
    if(ret){
        fprintf(stderr, "clCreateKernel: %s\n", clGetErrorString(ret));
        return 1;
    }

    // 5. 引数セット
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &found_seeds);
    if(ret){
        fprintf(stderr, "clSetKernelArg: %s\n", clGetErrorString(ret));
        return 1;
    }
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &found_count);
    if(ret){
        fprintf(stderr, "clSetKernelArg: %s\n", clGetErrorString(ret));
        return 1;
    }

    // 6. 実行 
    size_t global_size[2] = {W, W};
    long current_seed = 0L;
    size_t step = 1024;
    int host_found_count;
    long host_found_seeds[1000];
    int zero = 0;

    for(int i = 0; i < 1000; i++) {
        clEnqueueWriteBuffer(queue, found_count, CL_TRUE, 0, sizeof(int), &zero, 0, NULL, NULL);
        ret = clSetKernelArg(kernel, 0, sizeof(long), &current_seed);
        if(ret){
            fprintf(stderr, "clSetKernelArg: %s\n", clGetErrorString(ret));
            return 1;
        }
        ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &step, NULL, 0, NULL, NULL);
        if(ret){
            fprintf(stderr, "clEnqueueNDRangeKernel: %s\n", clGetErrorString(ret));
            return 1;
        }

        // 7. 結果取得
        ret = clEnqueueReadBuffer(queue, found_count, CL_TRUE, 0, sizeof(int), &host_found_count, 0, NULL, NULL);
        if(ret){
            fprintf(stderr, "clEnqueueReadBuffer: %s\n", clGetErrorString(ret));
            return 1;
        }

        // 結果表示 (見つかった場所だけ表示)
        if(host_found_count > 0)
        {
            printf("host found count: %d\n", host_found_count);
            ret = clEnqueueReadBuffer(queue, found_seeds, CL_TRUE, 0, sizeof(long) * (host_found_count > 1000 ? 1000 : host_found_count), host_found_seeds, 0, NULL, NULL);
            if(ret){
                fprintf(stderr, "clEnqueueReadBuffer: %s\n", clGetErrorString(ret));
                return 1;
            }
            for(int i = 0; i < host_found_count; i++)
            {
                printf("found seeds: %ld\n", host_found_seeds[i]);
            }
            break;
        }
        current_seed += step;
        if(i % 10 == 0) printf("Checked up to seed: %ld\n", current_seed);
    }
    return 0;
}
