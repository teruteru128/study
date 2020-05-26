#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# https://www.glamenv-septzen.net/view/488

AC_PREREQ([2.69])
AC_INIT([STUDY], [0.0.0-alpha], [teruterubouzu1024@gmail.com])
AC_CONFIG_SRCDIR([src/study.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIRS([m4])
AM_PROG_LIBTOOL
LT_INIT
AM_PROG_AR

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_GNU_SOURCE
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_YACC
AC_ENABLE_SHARED

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-lcurl':
AC_CHECK_LIB([curl], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])
# FIXME: Replace `main' with a function in `-lsodium':
AC_CHECK_LIB([sodium], [main])
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [main])
# FIXME: Replace `main' with a function in `-luuid':
AC_CHECK_LIB([uuid], [main])
# FIXME: Replace `main' with a function in `-ltextdomain':
AC_CHECK_LIB([textdomain], [main])
AC_CHECK_LIB([gmp], [__gmpz_init])
AC_CHECK_LIB([xmlrpc], [main])
AC_CHECK_LIB([xmlrpc_abyss], [main])
AC_CHECK_LIB([xmlrpc_client], [main])
AC_CHECK_LIB([xmlrpc_server], [main])
AC_CHECK_LIB([xmlrpc_server_abyss], [main])
AC_CHECK_LIB([xmlrpc_util], [main])
AC_CHECK_LIB([xmlrpc_xmlparse], [main])
AC_CHECK_LIB([xmlrpc_xmltok], [main])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([argz.h arpa/inet.h fcntl.h float.h inttypes.h langinfo.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdio_ext.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
m4_ifdef([AM_GNU_GETTEXT], [
AM_GNU_GETTEXT([external])
po_makefile_in=po/Makefile.in
AM_CONDITIONAL([HAVE_PO], [true])
],[
AM_CONDITIONAL([HAVE_PO], [false])
])
AC_FUNC_MALLOC
AC_FUNC_MBRTOWC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify getcwd gettimeofday localeconv mblen mbrlen memchr mempcpy memset munmap nl_langinfo pow regcomp select setlocale socket sqrt stpcpy strcasecmp strchr strcspn strdup strerror strrchr strstr strtol strtoul strtoull])

AC_CONFIG_FILES([Makefile
                 intl/Makefile
                 $po_makefile_in
                 src/Makefile])
AC_OUTPUT
