
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    char a[4] = "vim";
    char** b = NULL;
    int i;
    b = malloc(sizeof(char*) * 3);
    if(b == NULL){
        printf("NG");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < 3 ; i++){
        *(b + i) = a;
    }
    for(i = 0; i < 3 ; i++){
        printf("%s\n", *(b + i));
    }
    printf("\n");
    (*b)[0] = 'V';
    for(i = 0; i < 3 ; i++){
        printf("%s\n", *(b + i));
    }
    free(b);
    return 0;
}
