
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
    struct drand48_data data;
    uint64_t seeds[] = {125352706827826ULL, 116229385253865ULL};
    uint64_t seed;
    uint16_t seed2[3];
    size_t size = sizeof(struct drand48_data);
    for (int j = 0; j < 2; j++)
    {
        seed = seeds[j];
        fprintf(stderr, "before scramble: %012" PRIx64 "\n", seed);
        seed = initialScramble(seed);
        fprintf(stderr, "after scramble: %012" PRIx64 "\n", seed);
        // これメモリオーダーがビッグエンディアンだったら0x00ffff部分がコピーされるんか？
        memcpy(seed2, &seed, 6);
        fprintf(stderr, "after memcpy: %04" PRIx16 "%04" PRIx16 "%04" PRIx16 "\n", seed2[2], seed2[1], seed2[0]);
        seed48_r((uint16_t *)seed2, &data);
        fprintf(stderr, "after setseed: %04x%04x%04x\n", data.__x[2], data.__x[1], data.__x[0]);
        float f;
        int ffff;
        for (int i = 0; i < 2; i++)
        {
            f = nextFloat(&data);
            memcpy(&ffff, &f, 4);
            fprintf(stderr, "%f, %08x\n", f, ffff);
        }
        if (j != 1)
        {
            fprintf(stderr, "--\n");
        }
    }
    cl_int ret;

    // プラットフォーム数取得
    cl_uint num_platforms;
    if((ret = clGetPlatformIDs(0, NULL, &num_platforms)) != 0)
    {
        perror("clGetPlatformIDs");
        fprintf(stderr, "%s\n", clGetErrorString(ret));
        return 1;
    }
    printf("num_platforms: %u\n", num_platforms);

    // プラットフォーム取得
    cl_platform_id *platforms = alloca(sizeof(cl_platform_id) * num_platforms);
    if((ret = clGetPlatformIDs(num_platforms, platforms, NULL)) != 0)
    {
        perror("clGetPlatformIDs");
        fprintf(stderr, "%s\n", clGetErrorString(ret));
        return 1;
    }
    cl_platform_id platform = platforms[0];
    unsigned char *tmp = (unsigned char *)platform;
    for(size_t i = 0; i < 16; i++)
    {
        printf("%u", tmp[i] & 0xff);
    }
    printf("\n");

    // デバイス数取得
    cl_uint num_devices;
    if((ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices) != 0))
    {
        perror("clGetDeviceIDs");
        fprintf(stderr, "%s\n", clGetErrorString(ret));
        return 1;
    }
    printf("num_devices: %d\n", num_devices);

    // デバイス取得
    cl_device_id *devices = alloca(sizeof(cl_device_id) * num_devices);
    if((ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL) != 0))
    {
        perror("clGetDeviceIDs");
        fprintf(stderr, "%s\n", clGetErrorString(ret));
        return 1;
    }

    // コンテキスト作成
    cl_context context = clCreateContext(NULL, num_devices, devices, NULL, NULL, &ret);
    if(ret != CL_SUCCESS)
    {
        fprintf(stderr, "%s\n", clGetErrorString(ret));
		return 1;
    }
    if((ret = clReleaseContext(context)) != 0)
    {
        fprintf(stderr, "%s\n", clGetErrorString(ret));
        return 1;
    }
    return 0;
}
