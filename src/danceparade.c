
#include <stdio.h>

int main(int argc, char **argv, const char **envp)
{
    char *a[] = { "˘)ว", "˘ω˘)ว", "(ง˘ω˘)ว" };
    size_t full_face_count = 0;
    for (size_t i = 0; i < 27; i++)
    {
        printf("|");
        printf("%s", a[i%3]);
        full_face_count = i/3;
        for (size_t j = 0; j < full_face_count; j++)
        {
            printf("%s", a[2]);
        }
        
        printf("\n");
    }

    return 0;
}
