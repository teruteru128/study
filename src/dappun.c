
#include <stdio.h>

#define RIKO "ン゛ボップ"
#define PAKA "あああああああああああああああああああああああああああああああ！！！！！！！！！！！（ﾌﾞﾘﾌﾞﾘﾌﾞﾘﾌﾞﾘｭﾘｭﾘｭﾘｭﾘｭﾘｭ！！！！！！ﾌﾞﾂﾁﾁﾌﾞﾌﾞﾌﾞﾁﾁﾁﾁﾌﾞﾘﾘｲﾘﾌﾞﾌﾞﾌﾞﾌﾞｩｩｩｩｯｯｯ！！！！！！！ ）"
#define BUTTIPPA "ブッチッパ！"

int main(int argc, char **argv)
{
  for (int i = 65535; i; i--)
  {
    fputs(RIKO, stdout);
    fputs(PAKA, stdout);
    fputs(BUTTIPPA, stdout);
    fputs("\n", stdout);
  }
}
