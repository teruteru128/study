bin_PROGRAMS=study
bin_PROGRAMS+=003
bin_PROGRAMS+=004
bin_PROGRAMS+=main
bin_PROGRAMS+=005
bin_PROGRAMS+=006
bin_PROGRAMS+=007
bin_PROGRAMS+=008
bin_PROGRAMS+=010
bin_PROGRAMS+=011
bin_PROGRAMS+=012
bin_PROGRAMS+=013
bin_PROGRAMS+=014
bin_PROGRAMS+=015
bin_PROGRAMS+=016
bin_PROGRAMS+=017
bin_PROGRAMS+=018
bin_PROGRAMS+=019
bin_PROGRAMS+=020
bin_PROGRAMS+=021
bin_PROGRAMS+=022
bin_PROGRAMS+=orz
bin_PROGRAMS+=gettime
bin_PROGRAMS+=dappun
bin_PROGRAMS+=026
bin_PROGRAMS+=028
bin_PROGRAMS+=prime
bin_PROGRAMS+=prime2
bin_PROGRAMS+=calc
bin_PROGRAMS+=bouyomi
bin_PROGRAMS+=bm
bin_PROGRAMS+=udpreciever
bin_PROGRAMS+=udpsender
bin_PROGRAMS+=primesearch
bin_PROGRAMS+=portscanner
bin_PROGRAMS+=firstThread
bin_PROGRAMS+=firstThread2
bin_PROGRAMS+=firstThread3

#AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
study_CFLAGS= -I../include -DLOCALEDIR='"$(localedir)"'
#include_HEADERS= printint.h
study_LDADD= -lprintint -lcrypto
study_LDFLAGS= -L../lib
study_SOURCES=study.c\
							gettext-sample.c\
							gettext-sample.h
003_CFLAGS= -I../include
003_LDADD= -lprintint -lcrypto
003_LDFLAGS= -L../lib
005_CFLAGS= -I../include
005_LDADD= -lprintint -lssl -lcrypto
005_LDFLAGS= -L../lib
main_SOURCES=main.c base64.c
008_SOURCES=008.c
008_CFLAGS= -I../include
008_LDADD= -lprintint -lssl -lcrypto
008_LDFLAGS= -L../lib
010_SOURCES=010.c
015_LDFLAGS= -lcrypto
016_LDFLAGS= -lssl -lcrypto
017_CFLAGS= -I../include
017_LDADD= -lprintint -lssl -lcrypto
017_LDFLAGS= -L../lib
018_SOURCES= 018.c server.c
019_LDFLAGS=-lm
020_SOURCES= 020.c 020.h
022_SOURCES= 022.c 022.h
020_CFLAGS= -I../include -DLOCALEDIR='"$(localedir)"'
022_CFLAGS=-I../include -DLOCALEDIR='"$(localedir)"'
022_LDADD= -ltextdomain
022_LDFLAGS=-L../lib
026_SOURCES = 026.c 026.h
026_CFLAGS = -I../include -DLOCALEDIR='"$(localedir)"'
028_SOURCES = 028.c sha-1.c sha-2.c sha-3.c sha-n.c main028.h sha-1.h sha-2.h sha-3.h sha-n.h 028.h
028_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
bm_SOURCES = bitmessage.c base64.c bmapi.c xmlrpc.c bm.c bitmessage.h base64.h bmapi.h xmlrpc.h
bm_CFLAGS = `xmlrpc-c-config libwww-client --cflags`
bm_LDFLAGS = `xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
firstThread_LDFLAGS=-lpthread
firstThread2_LDFLAGS=-lpthread
firstThread3_LDFLAGS=-lpthread

