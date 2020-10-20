#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# https://www.glamenv-septzen.net/view/488

AC_PREREQ([2.69])
AC_INIT([study],[1.0.1],[teruterubouzu1024@gmail.com])
AC_CONFIG_SRCDIR([src/study.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIRS([m4])
AC_USE_SYSTEM_EXTENSIONS
AM_PROG_AR
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB([crypto], [main], [], [AC_MSG_FAILURE([could not find lib crypto])])
AC_CHECK_LIB([curl], [main], [], [AC_MSG_FAILURE([could not find lib curl])])
AC_CHECK_LIB([m], [main], [], [AC_MSG_FAILURE([could not find lib m])])
AC_CHECK_LIB([mysqlclient], [main], [], [AC_MSG_FAILURE([could not find lib mysqlclient])])
AC_CHECK_LIB([pthread], [main], [], [AC_MSG_FAILURE([could not find lib pthread])])
AC_CHECK_LIB([sodium], [main], [], [AC_MSG_FAILURE([could not find lib sodium])])
AC_CHECK_LIB([ssl], [main], [], [AC_MSG_FAILURE([could not find lib ssl])])
AC_CHECK_LIB([uuid], [main], [], [AC_MSG_FAILURE([could not find lib uuid])])
AC_CHECK_LIB([gmp], [__gmpz_init], [], [AC_MSG_FAILURE([could not find lib gmp])])
AC_CHECK_LIB([xmlrpc], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc])])
AC_CHECK_LIB([xmlrpc_abyss], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_abyss])])
AC_CHECK_LIB([xmlrpc_client], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_client])])
AC_CHECK_LIB([xmlrpc_server], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_server])])
AC_CHECK_LIB([xmlrpc_server_abyss], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_server_abyss])])
AC_CHECK_LIB([xmlrpc_util], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_util])])
AC_CHECK_LIB([xmlrpc_xmlparse], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_xmlparse])])
AC_CHECK_LIB([xmlrpc_xmltok], [main], [], [AC_MSG_FAILURE([could not find lib xmlrpc_xmltok])])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h endian.h fcntl.h float.h inttypes.h libintl.h limits.h locale.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_C_RESTRICT
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_GNU_GETTEXT([external])
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MBRTOWC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime getcwd gettimeofday inet_ntoa memchr memset pow setlocale socket sqrt strchr strdup strerror strpbrk strspn strstr strtoull])

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 po/Makefile.in
                 src/Makefile
                 src/config/Makefile
                 tests/Makefile])
AC_CONFIG_SUBDIRS([libstudy])
AC_OUTPUT
