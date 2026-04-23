
#include <stddef.h>
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
#include <curl/curl.h>
#include <dirent.h>
#include <endian.h>
#include <fcntl.h>
#include <gmp.h>
#include <iconv.h>
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
#include <stdatomic.h>
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

#include <dlfcn.h>
#include <plugin.h>

/**
 * @param argc
 * @param argv
 * @param envp
 * @return int
 */
int entrypoint(int argc, char **argv, char *const *envp)
{
    if(argc < 3)
    {
        fprintf(stderr, "%s <sofile> <function symbol>\n", argv[0]);
        return 1;
    }
        void *handle;
    feature_func func;
    char *error;

    printf("[Main] Loading optional feature...\n");

    // 1. 共有ライブラリをオープン (RTLD_LAZY: 必要になったらシンボル解決)
    handle = dlopen(argv[1], RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "[Main] No optional feature found: %s\n", dlerror());
        printf("[Main] Running without optional feature.\n");
        return 0;
    }

    // 2. シンボル(関数)の取得
    // dlerrorでエラークリアしてからdlsymを呼ぶのが定石
    dlerror();
    func = (feature_func)dlsym(handle, argv[2]);
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "[Main] Symbol not found: %s\n", error);
        dlclose(handle);
        return 1;
    }

    // 3. 関数の実行
    printf("[Main] Calling plugin function...\n");
    void *ret = func("Hello from dynamic load!");
    printf("[Main] Return pointer: %p\n", ret);

    // 4. クローズ
    dlclose(handle);
    printf("[Main] Optional feature closed.\n");
    return 0;
}
