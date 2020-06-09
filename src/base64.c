
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "base64.h"
#include <stdio.h>
static const char BASE64_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#define PAD ('=')

char *base64encode(const char *data, const size_t size)
{
  size_t length = 0;
  char *base64 = NULL;
  char *cursor = NULL;
  size_t i = 0;
  size_t j = 0;
  size_t k = 0;
  size_t unit = (size + 2) / 3;

  if (data == NULL)
  {
    return NULL;
  }

  length = size * 4 / 3 + 3 + 1;
#ifdef _DEBUG
  printf("size : %ld, length : %ld\n", size, length);
#endif
  base64 = malloc(length);
  if (base64 == NULL)
  {
    return NULL;
  }
  cursor = base64;
  for (i = 0, j = size, k = unit; k > 0; i += 3, j -= 3, k--)
  {
    if (j == 1)
    {
      *(cursor++) = BASE64_TABLE[(data[i + 0] >> 2 & 0x3f)];
      *(cursor++) = BASE64_TABLE[(data[i + 0] << 4 & 0x30)];
      *(cursor++) = PAD;
      *(cursor++) = PAD;
      //j += 2;
    }
    else if (j == 2)
    {
      *(cursor++) = BASE64_TABLE[(data[i + 0] >> 2 & 0x3f)];
      *(cursor++) = BASE64_TABLE[(data[i + 0] << 4 & 0x30) | (data[i + 1] >> 4 & 0x0f)];
      *(cursor++) = BASE64_TABLE[(data[i + 1] << 2 & 0x3c)];
      *(cursor++) = PAD;
      //j += 1;
    }
    else
    {
      *(cursor++) = BASE64_TABLE[(data[i + 0] >> 2 & 0x3f)];
      *(cursor++) = BASE64_TABLE[(data[i + 0] << 4 & 0x30) | (data[i + 1] >> 4 & 0x0f)];
      *(cursor++) = BASE64_TABLE[(data[i + 1] << 2 & 0x3c) | (data[i + 2] >> 6 & 0x03)];
      *(cursor++) = BASE64_TABLE[(data[i + 2] << 0 & 0x3f)];
    }
  }
  *cursor = '\0';
  return base64;
}
