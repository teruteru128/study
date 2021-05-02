
#include <stdio.h>
#include <math.h>

void eja()
{
    /* 
     * y=x/(1+x)
     * y(1+x)=x
     * y + xy = x
     * y=x-xy
     * y=x(1-y)
     * x=y/(1-y)
     * x=y/(4-y)
     * x(4-y) = y
     * 4x -xy = y
     * y + xy = 4x
     * y(1+x)=4x
     * y=4x/(1+x)
     * パラメータ一覧
     * - 棒とか玉のサイズ
     * - 快感の強さ
     * - 射精量スケール
     * - 安全リミット
     */
    double exp = 0;
    const double maxp = log10(2000);
    for (double i = 0; i <= 1000; i++)
    {
        exp = fmin(6 * (i / 100.0) / (2 + (i / 100.0)), maxp);
        printf("p : %f, %fL\n", exp, pow(10, exp) / 1000);
    }
}
