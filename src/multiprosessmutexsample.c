
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    size_t pagesize = sysconf(_SC_PAGESIZE);
    // pagesize単位で切り上げ, 切り上げ部分はマクロ化してもいいかも
    // size_t mapping_size = ((sizeof(pthread_mutex_t) + pagesize - 1) / pagesize) * pagesize;
    size_t mapping_size = sizeof(pthread_mutex_t);
    // プロセスをまたいだmutexとかはmmap必須なんですかね？
    pthread_mutex_t *mutex = mmap(NULL, mapping_size, PROT_READ | PROT_WRITE,
                                  MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (mutex == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }
    // pthreadをマルチプロセス下で使うには設定をする必要がある
    pthread_mutexattr_t mutexattr = { 0 };
    pthread_mutexattr_init(&mutexattr);
    if (pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) != 0)
    {
        perror("pthread_mutexattr_setpshared");
        return 1;
    }
    pthread_mutex_init(mutex, &mutexattr);
    // mutexのinitが終わったらattrは破棄してしまってもいいのだろうか？
    // "linuxの実装では"なにもしないので破棄してもしなくても変わらない
    // mutexからattrを参照してるかもしれないのが怖いんだよなぁ……
    pthread_mutexattr_destroy(&mutexattr);

    pid_t p = fork();
    if (p == -1)
    {
        perror("fork");
        return 1;
    }
    else if (p == 0)
    {
        // 子プロセス
        pthread_mutex_lock(mutex);
        printf("子プロセス\n");
        sleep(5);
        pthread_mutex_unlock(mutex);
        printf("子プロセスロック終わり\n");
        _exit(0);
    }
    else
    {
        // 親プロセス
        printf("親プロセスが子プロセスを先に行かせる...\n");
        sleep(1);
        int status = 0;
        printf("親プロセスがロックを試みる\n");
        pthread_mutex_lock(mutex);
        printf("親プロセス\n");
        pthread_mutex_unlock(mutex);
        printf("親プロセスロック終わり\n");
        wait(&status);
        printf("%d\n", status);
    }

    pthread_mutex_destroy(mutex);
    // pthread_mutexattr_destroy(&mutexattr);

    munmap(mutex, mapping_size);
    return 0;
}
