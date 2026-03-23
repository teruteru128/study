
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 1つの入力ファイルを読み込み、PARI/GPスクリプト形式で出力ファイルに書き込む
 */
size_t process_file(const char *filename, FILE *out_fp, size_t counter) {
    FILE *in_fp = fopen(filename, "r");
    if (!in_fp) {
        perror(filename);
        return counter;
    }

    char line[BUFSIZ];
    while (fgets(line, sizeof(line), in_fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        size_t digits = strlen(line);
        fprintf(out_fp,
                "write(\"certs/%04zu-cert%zu.txt\", primecertexport(primecert(%s), 1))\n",
                counter, digits, line);
        counter++;
    }

    fclose(in_fp);
    return counter;
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--append] [--counter N] [--outfile FILE] <input files...>\n", argv[0]);
        return 1;
    }

    int append = 0;
    size_t counter = 1;
    char outfile[PATH_MAX] = "gp-tests.txt";
    
    // 入力ファイルパスを一時的に保持する配列（簡易版）
    const char *input_files[argc];
    int input_count = 0;

    // 引数解析
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--append") == 0) {
            append = 1;
        } else if (strcmp(argv[i], "--counter") == 0 && i + 1 < argc) {
            counter = strtoull(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "--outfile") == 0 && i + 1 < argc) {
            snprintf(outfile, PATH_MAX, "%s", argv[++i]);
        } else {
            // オプション以外はすべて入力ファイルとして扱う
            input_files[input_count++] = argv[i];
        }
    }

    if (input_count == 0) {
        fprintf(stderr, "Error: No input files specified.\n");
        return 1;
    }

    // 出力ファイルオープン
    FILE *tests = fopen(outfile, append ? "a" : "w");
    if (!tests) {
        perror("Failed to open output file");
        return 1;
    }

    // ヘッダー書き込み（新規作成時のみ）
    if (!append) {
        fputs("default(nbthreads, 16);\n", tests);
        fputs("print(\"nbthreads->\", default(nbthreads), \"<-\");\n", tests);
    }

    // 各入力ファイルを順次処理
    size_t start_counter = counter;
    for (int i = 0; i < input_count; i++) {
        counter = process_file(input_files[i], tests, counter);
    }

    fclose(tests);
    fprintf(stderr, "Success: Processed %zu lines.\n", counter - start_counter);
    return 0;
}

