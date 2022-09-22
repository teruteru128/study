
#define _GNU_SOURCE
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <CL/opencl.h>
#include <errno.h>
#include <gmp.h>
#include <inttypes.h>
#include <math.h>
#include <netdb.h>
#include <omp.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

/*
 * 秘密鍵かな？
 * ioxhJc1lIE2m+WFdBg3ieQb6rk8sSvg3wRv/ImJz2tc=
 * cm2E2vmE0Nd8aVP/4Ph2S1R6C5bkC1H7CiUBzbcDG3U=
 * BixgbLYk35GP+XHYdK/DgSWIUXyCTwCwEtY4h/G22dw=
 * BH4RDmdo0Yq0Ftiw0lm9ej5BmpZ35kEw2kaWZlZ0Do8=
 * lMhxDh6RPpWOsnJMeS12pTJ/j7EPn+ugpdbNQCGbiwc=
 * 9hZn+KDlwjgrbyFpaX5ibuaO4QfaFbIL79NUrwJlcRQ=
 * T+tDF4I700WFkFhGieYxWgQKPO/MDcntDYzMrqQSZjzwV2DzaI1OM/CsJWE30WBqMI1SxbEQHufR1A76I7ayWN==
 * nySkaCQxGErccmiqLznSQduXgFICpjnl2bo7n3FAhQMlku79plIeL85/etpN865GAnlUpErSppEYHvn4couGh3==
 * ns2bQQ4zlnfcCTSAxEH3gDDYHcBswKw92jQeEgm+9tse74XdX+LNwgfw7OsMUjOGtLMb7R/kXNRXYv1AHi71iV==
 * NxhJ5JwWhUtUccCfJNtVqzdpCMGOaAtknmcEKLyglZFNXE66EiFi9wPFekwekx3ln8m9v5wnfv7V8jSrpZ/SHQ==
 * +3n5qDbtpicXBy+Yyol/TJkg2IoQ01vZ/U2SvgpP+Fdm4DrIYngY7X0ZS53rc/KKIHT//jVqNwNBz1sGFyYUDg==
 * cLtHGFI7X/Xl6Ly03DczMzl2bsHJmI2BMQKKCckUek5vTIiltDPfT3PxdT6zxW1LzwVqJIsQEkxxPNTswgpSFg==
 * pMQBNF+F12AXT3T0mQq7S0l1VcCr/Dw2Q54zeuHH0/1ExLgbhHEsmAHf3WR9nK/Ku1Mc/eU3vaAO78yplJB76A==
 * QUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQQ==
 * ↓2回連続getFloatで-1が出るseed 2つ
 * 125352706827826
 * 116229385253865
 * preforkする場合ってforkするのはlistenソケットを開く前？開いた後？
 */
int hiho(int argc, char **argv, const char **envp)
{
    // プラットフォーム取得
    cl_uint platformNumber = 0;
    cl_platform_id platformIds[8];
    clGetPlatformIDs(8, platformIds, &platformNumber);

    char string[256];
    cl_device_type type = 0;
    cl_uint value;
    size_t sizes[3];
    cl_ulong ulvalue;
    for (unsigned int i = 0; i < platformNumber; i++)
    {
        printf("platform idx : %d\n", i);
        cl_platform_id platform = platformIds[i];
        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 256, string, NULL);
        printf("platform vendor : %s\n", string);
        clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, string, NULL);
        printf("platform name : %s\n", string);
        clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, string, NULL);
        printf("platform version : %s\n", string);
        clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, 256, string, NULL);
        printf("platform extensions : %s\n", string);

        // デバイス取得
        cl_uint deviceNumber = 0;
        cl_device_id deviceIds[8];
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 8, deviceIds,
                       &deviceNumber);
        for (size_t j = 0; j < deviceNumber; j++)
        {
            cl_device_id device = deviceIds[j];
            clGetDeviceInfo(device, CL_DEVICE_NAME, 256, string, NULL);
            printf("    device name : %s\n", string);

            clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type),
                            &type, NULL);
            if (type != 0)
            {
                if ((type & CL_DEVICE_TYPE_DEFAULT) == CL_DEVICE_TYPE_DEFAULT)
                    printf("    device type : DEFAULT\n");
                if ((type & CL_DEVICE_TYPE_CPU) == CL_DEVICE_TYPE_CPU)
                    printf("    device type : CPU\n");
                if ((type & CL_DEVICE_TYPE_GPU) == CL_DEVICE_TYPE_GPU)
                    printf("    device type : GPU\n");
                if ((type & CL_DEVICE_TYPE_ACCELERATOR)
                    == CL_DEVICE_TYPE_ACCELERATOR)
                    printf("    device type : ACCELERATOR\n");
#ifdef CL_VERSION_1_2
                if ((type & CL_DEVICE_TYPE_CUSTOM) == CL_DEVICE_TYPE_CUSTOM)
                    printf("    device type : CUSTOM\n");
#endif
                if ((type & CL_DEVICE_TYPE_ALL) == CL_DEVICE_TYPE_ALL)
                    printf("    device type : ALL\n");
            }
            else
            {
                printf("    device type : EMPTY\n");
            }

            clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS,
                            sizeof(cl_uint), &value, NULL);
            printf("    device max compute units : %" PRIu32 "\n", value);

            clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES,
                            sizeof(size_t) * 3, sizes, NULL);
            printf("    device max work item sizes : [%zu][%zu][%zu]\n",
                   sizes[0], sizes[1], sizes[2]);

            clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                            sizeof(cl_uint), &value, NULL);
            printf("    device max work group size : %" PRIu32 "\n", value);

            clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                            sizeof(cl_ulong), &ulvalue, NULL);
            printf("    device max mem alloc size : %" PRIu64 "\n", ulvalue);

            clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
                            sizeof(cl_ulong), &ulvalue, NULL);
            printf("    device max constant buffer size : %" PRIu64 "\n",
                   ulvalue);
        }
    }
}
