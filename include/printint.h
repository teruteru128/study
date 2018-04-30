


#ifndef HEADER_PRINTINT_H
#define HEADER_PRINTINT_H

#ifdef  __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

size_t snprintUInt64(char * restrict, size_t, uint64_t);
size_t snprintInt64(char * restrict, size_t, int64_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
