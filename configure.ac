#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([STUDY], [0.0.0-alpha], [teruterubouzu1024@gmail.com])
AC_CONFIG_SRCDIR([include/config.h.in])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIRS([m4])
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-lprintint':
#AC_CHECK_LIB([printint], [main])
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [main])
# FIXME: Replace `main' with a function in `-luuid':
AC_CHECK_LIB([uuid], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h libintl.h locale.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.19)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memchr memset setlocale socket strerror strstr strtoull])
#LT_INIT

AC_CONFIG_FILES([001/Makefile 
                 003/Makefile
                 004/Makefile
                 005/Makefile
                 006/Makefile
                 007/Makefile
                 008/Makefile
                 010/Makefile
                 011/Makefile
                 012/Makefile
                 013/Makefile
                 014/Makefile
                 015/Makefile
                 016/Makefile
                 017/Makefile
                 018/Makefile
                 019/Makefile
                 020/Makefile
                 021/Makefile
                 022/Makefile
                 023/Makefile
                 024/Makefile
                 025/Makefile
                 po/Makefile.in
                 Makefile
                 lib/Makefile])
AC_OUTPUT

