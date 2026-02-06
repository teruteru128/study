
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
#define MAX_LINES 1000
#define MAX_LINE_LEN 8192

/**
 * curlで100件素数を取得する
 * pari/gpで証明書を生成する
 * libzipで圧縮する
 * curlでポストする
 */
int main(int argc, char const *argv[])
{
    pari_init(4ULL * 1024 * 1024 * 1024, 2);
    pari_sp av = avma;
    GEN n = gp_read_str("205418657260510546733082285384744539230409867551175649321847618486369671955948433884385977461705708629768431483165108678"
                        "842880244207056768833777064305266103662894985870100602095555059567859056442004466890122799106190021904948149190201604561"
                        "628294348810111247456668012661062759824237516660249492423275843900556024784633907707702168009570043412165798770398631403"
                        "96883441364071520209755306158275717559559480560005878211595554373620251722345004673");
    GEN cert = primecert(n, 0L);
    if (gequal0(cert))
    {
        pari_close();
        return 1;
    }
    GEN exportcert = primecertexport(cert, 1L);
    if (gequal0(exportcert))
    {
        pari_close();
        return 1;
    }
    char *str = GSTR(exportcert);
    fprintf(stdout, "%s\n", str);
    avma = av;
    pari_close();
    return 0;
}
