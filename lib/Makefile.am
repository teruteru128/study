lib_LTLIBRARIES = libsafe.la libprintint.la libtextdomain.la
#lib_LTLIBRARIES+= libsafe.so libprintint.so libtextdomain.so
#include_HEADERS= $(top_srcdir)/include/safe.h
AM_CPPFLAGS = -I$(top_srcdir)/include
libsafe_la_SOURCES=safe.c
libprintint_la_SOURCES=printint.c
libtextdomain_la_SOURCES=textdomain.c
libtextdomain_la_CFLAGS=-DLOCALEDIR='"$(localedir)"'
#libsafe_so_CFLAGS=-shared
#libsafe_so_SOURCES=safe.c
#libprintint_so_CFLAGS=-shared
#libprintint_so_SOURCES=printint.c
#libtextdomain_so_SOURCES=textdomain.c
#libtextdomain_so_CFLAGS=-DLOCALEDIR='"$(localedir)"' -shared

