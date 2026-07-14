
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
 *TODO: 引数・戻り値の構造体化 `run_logic(void)` だけでなく、初期化用関数と終了用関数も用意し、引数や戻り値に構造体のポインタを受け渡すようにすると、テストデータの流し込みが簡単になります。
 *TODO: ファイル監視と自動リロード `inotify`などを組み込み、`.so`ファイルが更新された瞬間に自動で`dlclose`->`dlopen`を繰り返すように設定すると、コードを修正してビルドするだけで即座に実行結果を確認できます。
 *TODO: メモリリークのチェック `valgrind`などを噛ませてこのローダーを実行することで、プラグイン側のコードにメモリリークがないかを厳密にテストできます。
 *TODO: クラッシュ分離(セグフォ対策) プラグイン側のバグで強制終了（セグメンテーション違反）してしまわないよう、fork() を使って別プロセスで .so をロード・実行させる設計にすると、サンドボックスとしての安全性が劇的に向上します。
 *
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
    void *main_handle;
    void *handle;
    feature_func func;
    char *error;

    printf("[Main] Loading optional feature...\n");

    // 1. 共有ライブラリをオープン (RTLD_LAZY: 必要になったらシンボル解決)
    error = dlerror();
    printf("initialize dlerror: %s\n", error);
    handle = dlopen(argv[1], RTLD_LAZY);
    if ((error = dlerror()) != NULL || !handle) {
        fprintf(stderr, "[Main] No optional feature found: %s\n", error);
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
