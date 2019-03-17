
lib_LTLIBRARIES=libsafe.la libprintint.la libtextdomain.la
include_HEADERS= $(top_srcdir)/include/safe.h
AM_CPPFLAGS = -I$(top_srcdir)/include
libsafe_la_SOURCES=safe.c
libprintint_la_SOURCES=printint.c
libtextdomain_la_SOURCES=textdomain.c
libtextdomain_la_CFLAGS := -DLOCALEDIR='"$(localedir)"'

