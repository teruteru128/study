
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * factordb_exportから数値を読み込んで桁数ごとに選り分ける
 */
int main(int argc, char *argv[])
{
    // 1. 引数チェック
    if (argc < 2)
    {
        fprintf(stderr, "エラー: 入力ファイルを指定してください。\n");
        return EXIT_FAILURE;
    }

    // 2. 入力ファイルのオープンと確認
    FILE *in = fopen(argv[1], "r");
    if (in == NULL)
    {
        perror("入力ファイルのオープンに失敗しました");
        return EXIT_FAILURE;
    }

    char buffer[BUFSIZ];
    FILE *out = NULL;
    size_t currentDigits = 0;
    size_t prevDigits = 0;
    char name[NAME_MAX + 1]; // 終端文字分を確保

    while (fgets(buffer, sizeof(buffer), in))
    {
        // 改行文字の除去
        buffer[strcspn(buffer, "\r\n")] = '\0';
        
        // 空行はスキップ
        if (buffer[0] == '\0') 
        {
            continue;
        }

        currentDigits = strlen(buffer);

        // 桁数が変わった場合のみファイルを切り替える
        if (currentDigits != prevDigits)
        {
            if (out != NULL)
            {
                fclose(out);
                out = NULL;
            }

            // 安全な文字列バッファ構築
            int written = snprintf(name, sizeof(name), "factordb_export_%zudigits.txt", currentDigits);
            if (written < 0 || (size_t)written >= sizeof(name)) 
            {
                fprintf(stderr, "エラー: ファイル名が長すぎます。\n");
                continue;
            }

            fprintf(stderr, "opening %s...\n", name);
            out = fopen(name, "a");
            if (out == NULL)
            {
                perror("出力ファイルのオープンに失敗しました");
                prevDigits = 0; // 次のループで再試行させるためリセット
                continue;
            }
        }

        // 出力ファイルが正常に開いている場合のみ書き込み
        if (out != NULL)
        {
            fprintf(out, "%s\n", buffer);
        }
        
        prevDigits = currentDigits;
    }

    // 後処理
    if (out != NULL)
    {
        fclose(out);
    }
    fclose(in);

    return EXIT_SUCCESS;
}

