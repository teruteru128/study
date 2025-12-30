
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <pari/pari.h>
#define t_INT t_INT
#define t_REAL t_REAL
#define t_INTMOD t_INTMOD
#define t_FRAC t_FRAC
#define t_FFELT t_FFELT
#define t_COMPLEX t_COMPLEX
#define t_PADIC t_PADIC
#define t_QUAD t_QUAD
#define t_POLMOD t_POLMOD
#define t_POL t_POL
#define t_SER t_SER
#define t_RFRAC t_RFRAC
#define t_QFB t_QFB
#define t_VEC t_VEC
#define t_COL t_COL
#define t_MAT t_MAT
#define t_LIST t_LIST
#define t_STR t_STR
#define t_VECSMALL t_VECSMALL
#define t_CLOSURE t_CLOSURE
#define t_ERROR t_ERROR
#define t_INFINITY t_INFINITY

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "ファイルを指定してください\n");
        return EXIT_FAILURE;
    }
    FILE *fin, *fout;
    char line[8192];

    // PARIスタックの初期化 (約100MBを確保)
    pari_init(17179869184ULL, 1024);

    // 入力ファイルと出力ファイルのオープン
    fin = fopen(argv[1], "r");
    fout = fopen("certificates.txt", "wa");
    if (!fin || !fout)
    {
        fprintf(stderr, "ファイルのオープンに失敗しました。\n");
        return 1;
    }

    // 1行ずつ読み込んで処理
    while (fgets(line, sizeof(line), fin))
    {
        pari_sp av = avma;
        line[strcspn(line, "\r\n")] = '\0';
        fprintf(stderr, "%zu桁\n", strlen(line));
        // PARIの整数オブジェクト(GEN)に変換
        GEN n = gp_read_str(line);

        // 素数かどうかを確認
        if (gisprime(n, 0))
        {
            // 素数証明書の生成
            GEN cert = primecert(n, 0);

            // 文字列として書き出し
            GEN certstr = primecertexport(cert, 1);
            pari_fprintf(fout, "%Ps\n", certstr);
            char *a = pari_sprintf("%Ps\n", certstr);
            malloc(a);
        }
        else
        {
            fprintf(fout, "Number: %s Status: Not Prime\n\n", line);
        }

        avma = av;
    }

    fclose(fin);
    fclose(fout);
    pari_close();
    return 0;
}
