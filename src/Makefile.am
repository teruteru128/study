SUBDIRS = config

bin_PROGRAMS=main

CLEANFILES=*.aux *.dvi *.ps *.pdf

test.pdf: test.tex

.ps.pdf:
	ps2pdf $<

.dvi.ps:
	dvips $<

.tex.div:
	platex $<

studydatadir = $(datadir)/study

# see https://www.gnu.org/software/automake/manual/html_node/Subpackages.html
AM_CPPFLAGS=-Wall \
    -Wextra \
    -pedantic-errors \
    -Wcast-align \
    -Wcast-qual \
    -Wconversion \
    -Wdisabled-optimization \
    -Wendif-labels \
    -Wfloat-equal \
    -Winit-self \
    -Winline \
    -Wlogical-op \
    -Wmissing-include-dirs \
    -Wpacked \
    -Wpointer-arith \
    -Wredundant-decls \
    -Wshadow \
    -Wswitch-default \
    -Wswitch-enum \
    -Wunsafe-loop-optimizations \
    -Wvariadic-macros \
    -Wwrite-strings \
    -I$(top_srcdir)/libstudy/include \
    -DLOCALEDIR='"\"$(localedir)\""' \
    -DSTUDYDATADIR='"\"$(studydatadir)\""'

main_SOURCES=main.cpp
