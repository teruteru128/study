#include <stdio.h>

//Joinを1関数で完結させるのはまずいような？
char *strjoin(char *delimiter, char **array, size_t arraylen)
{
  // delimiterがNULLのときの考慮は？ ->
  // delimiterを別の変数に移し替えてNULLの場合は空文字列にする
  char *deli = delimiter;
  if (deli == NULL)
  {
    deli = "";
  }
  size_t deli_len = strlen(deli);
  size_t strcap = 0;
  int i = 0;
  char *rtnstr = NULL;

  if (arraylen == 0 || array == NULL)
  {
    rtnstr = calloc(1, 1);
    return rtnstr;
  }
  // sum array texts
  for (i = 0; i < arraylen; i++)
  {
    strcap += strlen(array[i]);
  }
  // sum used delimiter
  if (arraylen > 1)
  {
    strcap += (arraylen - 1) * deli_len;
  }
  // sum capacity for string termination(\0)
  strcap += 1;

  rtnstr = calloc(strcap, sizeof(char));
}
