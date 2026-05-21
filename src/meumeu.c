
#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int main(int argc, char *argv[])
{
    wchar_t hiragana[] = L"あいうえおかがきぎくぐけげこごさざしじすずせぜそぞただちぢつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもやゆよらりるれろわゐゑをんゔ";
    int j;
    setlocale(LC_ALL, "");
    size_t length = wcslen(hiragana);
    for(int i = 0; i < length; i++)
    {
        for(j = 0; j < length; j++)
        {
            printf("めうめう%1$lcっ%2$lcん%2$lcん！！\n", hiragana[i], hiragana[j]);
        }
    }

    return 0;
}

