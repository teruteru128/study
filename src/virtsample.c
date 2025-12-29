
#include <stdio.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    size_t size = 127ULL * 1024 * 1024 * 1024; // 64GB

    // 仮想メモリ領域の確保 (物理メモリは消費しない)
    void *ptr = mmap(NULL, size, PROT_NONE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    printf("127GBの仮想メモリを確保しました。ptr: %p\n", ptr);
    printf("topコマンドでVIRTを確認してください。終了するにはEnter...\n");
    fgetc(stdin);

    munmap(ptr, size);
    return 0;
}
