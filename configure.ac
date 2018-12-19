#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# https://www.glamenv-septzen.net/view/488

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
# FIXME: Replace `main' with a function in `-lprintint':
#AC_CHECK_LIB([printint], [main])
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [SSL_library_init])
AC_CHECK_LIB([crypto], [EVP_EncryptInit])
AX_CHECK_OPENSSL
if test "$with_openssl" = yes ; then
  dnl Order matters!
  if test "$PORTNAME" != "win32"; then
     AC_CHECK_LIB(crypto, CRYPTO_new_ex_data, [], [AC_MSG_ERROR([library 'crypto' is required for OpenSSL])])
     FOUND_SSL_LIB="no"
     AC_CHECK_LIB(ssl, OPENSSL_init_ssl, [FOUND_SSL_LIB="yes"])
     AC_CHECK_LIB(ssl, SSL_library_init, [FOUND_SSL_LIB="yes"])
     AS_IF([test "x$FOUND_SSL_LIB" = xno], [AC_MSG_ERROR([library 'ssl' is required for OpenSSL])])
  else
     AC_SEARCH_LIBS(CRYPTO_new_ex_data, eay32 crypto, [], [AC_MSG_ERROR([library 'eay32' or 'crypto' is required for OpenSSL])])
     FOUND_SSL_LIB="no"
     AC_SEARCH_LIBS(OPENSSL_init_ssl, ssleay32 ssl, [FOUND_SSL_LIB="yes"])
     AC_SEARCH_LIBS(SSL_library_init, ssleay32 ssl, [FOUND_SSL_LIB="yes"])
     AS_IF([test "x$FOUND_SSL_LIB" = xno], [AC_MSG_ERROR([library 'ssleay32' or 'ssl' is required for OpenSSL])])
  fi
fi
# FIXME: Replace `main' with a function in `-luuid':
AC_CHECK_LIB([uuid], uuid_generate)
# FIXME: Replace `main' with a function in `-ltextdomain':
AC_CHECK_LIB([textdomain], [main])
AC_CHECK_LIB(gmp, __gmpz_init)
AC_CHECK_LIB(xmlrpc, [main])
AC_CHECK_LIB(xmlrpc_abyss, [main])
AC_CHECK_LIB(xmlrpc_client, [main])
AC_CHECK_LIB(xmlrpc_server, [main])
AC_CHECK_LIB(xmlrpc_server_abyss, [main])
AC_CHECK_LIB(xmlrpc_util, [main])
AC_CHECK_LIB(xmlrpc_xmlparse, [main])
AC_CHECK_LIB(xmlrpc_xmltok, [main])

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
AC_CANONICAL_HOST
#AM_GNU_SOURCE
AM_GNU_GETTEXT
AM_GNU_GETTEXT_VERSION([0.19.8])
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memchr memset setlocale socket strerror strstr strtoull])
#LT_INIT

AC_CONFIG_FILES([src/Makefile
                 001/Makefile
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
                 026/Makefile
                 027/Makefile
                 028/Makefile
                 029/Makefile
                 030/Makefile
                 031/Makefile
                 032/Makefile
                 033/Makefile
                 034/Makefile
                 036/Makefile
                 037/Makefile
                 038/Makefile
                 039/Makefile
                 040/Makefile
                 041/Makefile
                 043/Makefile
                 044/Makefile
                 045/Makefile
                 046/Makefile
                 047/Makefile
                 048/Makefile
                 049/Makefile
                 050/Makefile
                 051/Makefile
                 052/Makefile
                 053/Makefile
                 054/Makefile
                 intl/Makefile
                 po/Makefile.in
                 Makefile
                 lib/Makefile])
AC_OUTPUT
