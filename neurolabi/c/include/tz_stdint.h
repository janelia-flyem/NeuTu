#ifndef _TZ_STDINT_H_
#define _TZ_STDINT_H_

#include "neurolabi_config.h"

#  include <stdint.h>

#    ifdef HAVE_SYS_TYPES_H
#      include <sys/types.h>
#    endif

#if !defined(_SSIZE_T) && !defined(__ssize_t_defined) && !defined(_WIN64) && !defined(_WIN32) && !defined(_SSIZE_T_)
#define _SSIZE_T
#define __ssize_t_defined
#  if !defined(ARCH_64)
typedef int32_t ssize_t;
#  else
typedef int64_t ssize_t;
#  endif
#endif

#if defined(_MSC_VER)
#ifdef _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif

typedef int8_t byte_t;
typedef int16_t word_t;

#if !defined(HAVE_BZERO)
#define bzero(dest,count) memset(dest,0,count)
#endif

#ifndef _UINT64_T
#define _UINT64_T
typedef unsigned long long   uint64_t;
#endif /* _UINT64_T */

/*
#if !defined(INT_MAX)
#define INTMAX 2147483647
#endif

#if !defined(INT_MIN)
#define INT_MIN -2147483648
#endif
*/

#endif
