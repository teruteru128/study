
#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        return EXIT_FAILURE;
    }
    FILE *in = NULL;
    char buf[8192];
    size_t length = 0;
    size_t count = 0;

    const char **input_files = alloca(sizeof(char *) * argc);
    int input_count = 0;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--apend") == 0) {
            // append = 1;
        } else if(strcmp(argv[i], "--offset-count") == 0 && i + 1 < argc) {
            count = strtoull(argv[++i], NULL, 10);
        } else if( strcmp(argv[i], "--outfile") == 0 && i + 1 < argc){
            //snprintf();
        } else {
            input_files[input_count++] = argv[i];
        }
    }

    for(int i = 1; i < argc; i++) {
        in = fopen(argv[i], "r");
        if(!in) {
            perror("!");
        }

        while(fgets(buf, 8192, in) != NULL) {
           buf[strcspn(buf, "\r\n")] = '\0';
           length = strlen(buf); 
           printf("mpirun -np 8 ecpp-mpi -g -t -c -f %zu-cert%zu -n %s\n", count, length, buf);
           count++;
        }

        fclose(in);
    }

    return EXIT_SUCCESS;
}
