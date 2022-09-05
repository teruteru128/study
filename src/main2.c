
#define _GNU_SOURCE
#define OPENSSL_API_COMPAT 0x30000000L
#define OPENSSL_NO_DEPRECATED 1

#include <CL/cl.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <openssl/bn.h>
#include <openssl/opensslv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// OpenCLのデバイス情報を管理するクラスを作成します
typedef struct ___opencl_device
{
    cl_platform_id platform_id; // PlatformのID
    cl_device_id device_id;     // DeviceのID
    cl_int num_of_platforms;    // システムが保持しているPlatform数
    cl_int num_of_devices;      // システムが保持しているDevice数
} OpenCLDevice;
// OpenCLコンテキストを管理するクラスです
typedef struct ___opencl_context
{
    cl_context context;     // OpenCLコンテキスト
    OpenCLDevice *p_device; // 関連するデバイス
} OpenCLContext;
// OpenCLコマンドキュー情報を管理するクラス
typedef struct ___opencl_queue
{
    cl_command_queue command_queue; // OpenCLコマンドキュー
    OpenCLDevice *p_device;         // 関連するデバイス
    OpenCLContext *p_context;       // 関連するコンテキスト
} OpenCLQueue;
typedef struct ___opencl_buffer
{
    cl_mem memobj;            // OpenCLメモリオブジェクト
    OpenCLContext *p_context; // 関連するコンテキスト

    char *p_buffer; // メモリオブジェクトを経由して情報をやりとりするメモリ
    size_t size;    // p_bufferのサイズ
} OpenCLBuffer;
typedef struct ___opencl_kernel
{
    cl_program program; // OpenCLプログラムオブジェクト
    cl_kernel kernel;   // OpenCLカーネルオブジェクト

    OpenCLDevice *p_device; // 関連するデバイス

    OpenCLContext context; // カーネル用のコンテキスト
    OpenCLQueue queue;     // カーネル用のコマンドキュー

    char *p_kernel_name; // カーネル名
} OpenCLKernel;

void OpenCLDevice_Init(OpenCLDevice *p_this)
{
    p_this->platform_id = NULL;
    p_this->device_id = NULL;
    p_this->num_of_platforms = 0;
    p_this->num_of_devices = 0;
}

int OpenCLDevice_GetDevices(OpenCLDevice *p_this)
{
    cl_int ret;
    // プラットフォームIDを取得します
    ret = clGetPlatformIDs(1, &(p_this->platform_id),
                           &(p_this->num_of_platforms));

    // プラットフォームが持っているデバイスIDとデバイス数を取得します
    ret = clGetDeviceIDs(p_this->platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
                         &(p_this->device_id), &(p_this->num_of_devices));

    // retにはOpenCLが提供するエラー番号が格納されます
    // 成功した場合は、CL_SUCCESSが格納されます

    // OpenCLのエラー番号を返却してもよいですが、
    // ここではシステムやプロジェクトで定義されているエラー定義を返しています
    return EXIT_SUCCESS;
}

void OpenCLContext_Init(OpenCLContext *p_this, OpenCLDevice *p_dev)
{
    p_this->context = NULL;
    p_this->p_device = p_dev; // 関連するデバイスへの参照を保存
}

int OpenCLContext_Create(OpenCLContext *p_this)
{
    cl_int ret;
    // コンテキストを取得します
    p_this->context = clCreateContext(NULL, 1, &(p_this->p_device->device_id),
                                      NULL, NULL, &ret);

    return EXIT_SUCCESS;
}

int OpenCLContext_Finalize(OpenCLContext *p_this)
{
    cl_int ret;
    // コンテキストをリリースします
    ret = clReleaseContext(p_this->context);

    return EXIT_SUCCESS;
}

void OpenCLQueue_Init(OpenCLQueue *p_this, OpenCLDevice *p_dev,
                      OpenCLContext *p_ctx)
{
    p_this->command_queue = NULL;
    p_this->p_device = p_dev;
    p_this->p_context = p_ctx;
}

int OpenCLQueue_Create(OpenCLQueue *p_this)
{
    cl_int ret;

    // コマンドキューを作成します
    p_this->command_queue = clCreateCommandQueue(
        p_this->p_context->context, p_this->p_device->device_id, 0, &ret);

    return EXIT_SUCCESS;
}

int OpenCLQueue_Read(OpenCLQueue *p_this, const OpenCLBuffer *p_rbuff)
{
    cl_int ret;

    // コマンドキューからデータを読み込みます
    // 読み込むために、OpenCLのバッファオブジェクトを利用します
    clEnqueueReadBuffer(p_this->command_queue, p_rbuff->memobj, CL_TRUE, 0,
                        p_rbuff->size, p_rbuff->p_buffer, 0, NULL, NULL);

    return EXIT_SUCCESS;
}

int OpenCLQueue_Finalize(OpenCLQueue *p_this)
{
    cl_int ret;

    // コマンドキューをリリースします
    ret = clFlush(p_this->command_queue);
    ret = clFinish(p_this->command_queue);
    ret = clReleaseCommandQueue(p_this->command_queue);

    return EXIT_SUCCESS;
}

void OpenCLBuffer_Init(OpenCLBuffer *p_this, OpenCLContext *p_ctx)
{
    p_this->memobj = NULL;
    p_this->p_context = p_ctx;

    p_this->p_buffer = NULL;
    p_this->size = 0;
}

int OpenCLBuffer_Create(OpenCLBuffer *p_this, size_t size)
{
    cl_int ret;

    // メモリオブジェクトを作成します
    p_this->memobj = clCreateBuffer(p_this->p_context->context,
                                    CL_MEM_READ_WRITE, size, NULL, &ret);

    // 後にメモリオブジェクトに渡すホスト側のメモリ領域を確保します
    p_this->p_buffer = malloc(size);
    p_this->size = size;

    return EXIT_SUCCESS;
}

int OpenCLBuffer_Finalize(OpenCLBuffer *p_this)
{
    cl_int ret;

    // メモリオブジェクトをリリースします
    ret = clReleaseMemObject(p_this->memobj);

    // mallocで格納したホスト側のメモリも解放します
    free(p_this->p_buffer);

    return EXIT_SUCCESS;
}

void OpenCLKernel_Init(OpenCLKernel *p_this, OpenCLDevice *p_dev)
{
    p_this->program = NULL;
    p_this->kernel = NULL;
    p_this->p_device = p_dev;

    p_this->p_kernel_name = NULL;
}

int OpenCLKernel_Setup(OpenCLKernel *p_this)
{
    cl_int ret;

    // 変数を初期化します
    OpenCLContext_Init(&(p_this->context), p_this->p_device);
    OpenCLQueue_Init(&(p_this->queue), p_this->p_device, &(p_this->context));

    // コンテキストとコマンドキューを作成
    OpenCLContext_Create(&(p_this->context));
    OpenCLQueue_Create(&(p_this->queue));

    return EXIT_SUCCESS;
}

int OpenCLKernel_CreateWithSource(OpenCLKernel *p_this, const char *p_src,
                                  size_t src_size)
{
    cl_int ret;

    // カーネルソースコードからプログラムオブジェクトを作成します
    p_this->program = clCreateProgramWithSource(
        p_this->context.context, 1, (const char **)&p_src,
        (const size_t *)&src_size, &ret);

    // プログラムをビルドします
    ret = clBuildProgram(p_this->program, 1, &(p_this->p_device->device_id),
                         NULL, NULL, NULL);

    // ビルド結果を表示します
    if (ret != CL_SUCCESS)
    {
        printf("Build Failure:%s\n", p_this->p_kernel_name);
    }
    else
    {
        printf("Build EXIT_SUCCESS:%s\n", p_this->p_kernel_name);
    }

    // カーネルオブジェクトを作成します
    p_this->kernel
        = clCreateKernel(p_this->program, p_this->p_kernel_name, &ret);

    return EXIT_SUCCESS;
}

int OpenCLKernel_Execute(OpenCLKernel *p_this, OpenCLBuffer *p_args, int len)
{
    cl_int ret;

    // 並列数を指定するための変数です
    // この指定は最もシンプルな単一ワークアイテムのみのカーネルで実行する設定です
    cl_uint work_dim = 1;
    size_t global_work_size[] = { 1 };
    size_t local_work_size[] = { 1 };

    // カーネル関数の左側から順番に引数を設定していきます
    int i;
    for (i = 0; i < len; i++)
    {
        ret = clSetKernelArg(p_this->kernel, i, sizeof(cl_mem),
                             &(p_args[i].memobj));
    }

    // カーネルを実行します
    ret = clEnqueueNDRangeKernel(p_this->queue.command_queue, p_this->kernel,
                                 work_dim, NULL, global_work_size,
                                 local_work_size, 0, NULL, NULL);

    // カーネルの実行結果を取得します
    // カーネル関数の第一引数が戻り値であることを前提にしています
    OpenCLQueue_Read(&(p_this->queue), &(p_args[0]));

    return EXIT_SUCCESS;
}

int OpenCLKernel_Finalize(OpenCLKernel *p_this)
{
    cl_int ret;

    // コンテキストとコマンドキューオブジェクトをリリース
    OpenCLContext_Finalize(&(p_this->context));
    OpenCLQueue_Finalize(&(p_this->queue));

    // カーネルオブジェクトをリリース
    ret = clReleaseKernel(p_this->kernel);

    // プログラムオブジェクトをリリース
    ret = clReleaseProgram(p_this->program);

    return EXIT_SUCCESS;
}
static char g_src[] = "__kernel void hello(__global char* string){\n"
                      "string[ 0] = 'H';\n"
                      "string[ 1] = 'e';\n"
                      "string[ 2] = 'l';\n"
                      "string[ 3] = 'l';\n"
                      "string[ 4] = 'o';\n"
                      "string[ 5] = ',';\n"
                      "string[ 6] = 'w';\n"
                      "string[ 7] = 'o';\n"
                      "string[ 8] = 'r';\n"
                      "string[ 9] = 'l';\n"
                      "string[10] = 'd';\n"
                      "string[11] = '.';\n"
                      "string[12] = '\\n';\n"
                      "string[13] = '\\0';\n"
                      "}\n";
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
    OpenCLDevice device; // デバイス情報を管理します
    OpenCLKernel kernel; // カーネル情報を管理します
    OpenCLBuffer
        arg; // カーネルに渡す引数の情報を格納するメモリオブジェクトです

    // デバイス情報の取得
    OpenCLDevice_Init(&device);
    OpenCLDevice_GetDevices(&device);

    // カーネルコードのビルド
    OpenCLKernel_Init(&kernel, &device);
    OpenCLKernel_Setup(&kernel);
    kernel.p_kernel_name = "hello";
    OpenCLKernel_CreateWithSource(&kernel, g_src, sizeof(g_src));

    // 引数に渡すバッファを作成
    OpenCLBuffer_Create(&arg, 32);

    // カーネルを実行
    OpenCLKernel_Execute(&kernel, &arg, 1);

    // カーネルからの戻り値を表示
    printf("from device=%s", arg.p_buffer);

    // 作成したオブジェクトをリリース
    OpenCLKernel_Finalize(&kernel);
    OpenCLBuffer_Finalize(&arg);

    return 0;
}
