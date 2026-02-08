
#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#ifdef CL_TARGET_OPENCL_VERSION
#include <CL/cl.h>
#include <CL/opencl.h>
#endif
#include <bm_sonota.h>
#include <complex.h>
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <gmp.h>
#include <iconv.h>
#include <inttypes.h>
#include <java_random.h>
#include <jsonrpc-glib.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <netdb.h>
#include <omp.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <png.h>
#include <printaddrinfo.h>
#include <regex.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <alloca.h>

#include <jpeglib.h> // jpeglibはstdioより下(FILEが依存しているため)

#if OPENSSL_VERSION_PREREQ(3, 0)
#include <openssl/core_names.h>
#include <openssl/param_build.h>
#include <openssl/provider.h>
#include <openssl/types.h>
#endif

#include "ripemd160.h"

const char *clGetErrorString(cl_int err) {
    switch (err) {
        // 基本的なランタイムエラー
        case CL_SUCCESS:                            return "Success!";
        case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
        case CL_DEVICE_NOT_AVAILABLE:               return "Device not available.";
        case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available.";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure.";
        case CL_OUT_OF_RESOURCES:                   return "Out of resources.";
        case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory.";
        case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available.";
        case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap.";
        case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch.";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported.";
        case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure.";
        case CL_MAP_FAILURE:                        return "Map failure.";
        case CL_INVALID_VALUE:                      return "Invalid value.";
        case CL_INVALID_DEVICE_TYPE:                return "Invalid device type.";
        case CL_INVALID_PLATFORM:                   return "Invalid platform.";
        case CL_INVALID_DEVICE:                     return "Invalid device.";
        case CL_INVALID_CONTEXT:                    return "Invalid context.";
        case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties.";
        case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue.";
        case CL_INVALID_HOST_PTR:                   return "Invalid host pointer.";
        case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object.";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor.";
        case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size.";
        case CL_INVALID_SAMPLER:                    return "Invalid sampler.";
        case CL_INVALID_BINARY:                     return "Invalid binary.";
        case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options.";
        case CL_INVALID_PROGRAM:                    return "Invalid program.";
        case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable.";
        case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name.";
        case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition.";
        case CL_INVALID_KERNEL:                     return "Invalid kernel.";
        case CL_INVALID_ARG_INDEX:                  return "Invalid argument index.";
        case CL_INVALID_ARG_VALUE:                  return "Invalid argument value.";
        case CL_INVALID_ARG_SIZE:                   return "Invalid argument size.";
        case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments.";
        case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension.";
        case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size.";
        case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size.";
        case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset.";
        case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list.";
        case CL_INVALID_EVENT:                      return "Invalid event.";
        case CL_INVALID_OPERATION:                  return "Invalid operation.";
        case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object.";
        case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size.";
        case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-level.";
        case CL_INVALID_GLOBAL_WORK_SIZE:           return "Invalid global work size.";
        case CL_INVALID_PROPERTY:                   return "Invalid property.";
        default:                                    return "Unknown OpenCL error.";
    }
}

// https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
/**
 * @brief
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * ↓getDoubleで可能な限り1に近い値が出るseed
 * 155239116123415
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 * ハッシュの各バイトを１バイトにORで集約して結果が0xffにならなかったら成功
 * 丸数字の1から50までforで出す
 * timer_create+sigeventでタイマーを使って呼ばれたスレッドから新しくスレッドを起動する
 *
 * decodable random source?
 *
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int entrypoint(int argc, char **argv, char *const *envp)
{
    if(argc < 2)
    {
        return 1;
    }
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
    FILE *in = fopen(argv[1], "r");
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
