
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

/**
 * @brief みかか方式暗号？
 * 
 * @return int 
 */
int main(void)
{
    const wchar_t in[] = L"はくらりしひくたひくりのし\nなりすこすなり\nますひ"
                         L"しり\nてすひりさく";
    const wchar_t a[] = L"ぬふあうえおやゆよわほへー"
                        L"たていすかんなにらせ゛゜"
                        L"ちとしはきくまのりれけむ"
                        L"つさそひこみもねるめろ";
    const char b[] = "1234567890-^\\qwertyuiop@[asdfghjkl;:]zxcvbnm,./\\";

    size_t max_len = wcslen(in);
    size_t j = 0;
    for (size_t i = 0; i < max_len; i++)
    {
        if (in[i] == L'\n')
        {
            wprintf(L"\n");
            continue;
        }
        for (j = 0; j < 49; j++)
        {
            if (in[i] == a[j])
            {
                break;
            }
        }
        wprintf(L"%c", (b[j]-3) < 'a' ? (b[j]-3) + 26:(b[j]-3));
    }
    wprintf(L"\n");
    return EXIT_SUCCESS;
}
