
#include <stdio.h>

int main(int argc, char **argv)
{
  char *riko = "ン゛ボップ";
  char *paka = "あああああああああああああああああああああああああああああああ！！！！！！！！！！！（ﾌﾞﾘﾌﾞﾘﾌﾞﾘﾌﾞﾘｭﾘｭﾘｭﾘｭﾘｭﾘｭ！！！！！！ﾌﾞﾂﾁﾁﾌﾞﾌﾞﾌﾞﾁﾁﾁﾁﾌﾞﾘﾘｲﾘﾌﾞﾌﾞﾌﾞﾌﾞｩｩｩｩｯｯｯ！！！！！！！ ）";
  char *buttippa = "ブッチッパ！";
  for (int i = 65535; i; i--)
  {
    fputs(riko, stdout);
    fputs(paka, stdout);
    fputs(buttippa, stdout);
    fputs("\n", stdout);
  }
}
