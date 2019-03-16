bin_PROGRAMS=study\
						003\
						004\
						main\
						005\
						006\
						007\
						008\
						010\
						011\
						012\
						013\
						014\
						015\
						016\
						017\
						018\
						019\
						020\
						021\
						022\
						orz\
						gettime\
						dappun\
						026\
						028\
						prime\
						prime2\
						calc\
						bouyomi\
						udpreciever\
						udpsender\
						primesearch\
						portscanner\
						rsa1024

cudahello:

%:%.cu
	nvcc -ccbin clang-4.0 $^ -o $@

#AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
study_CFLAGS= -I../include -DLOCALEDIR='"$(localedir)"'
#include_HEADERS= printint.h
study_LDADD= -lprintint -lcrypto
study_LDFLAGS= -L../lib
study_SOURCES=study.c\
							001-gettext-sample.c\
							001.h
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
012_LDFLAGS= -luuid
015_LDFLAGS= -lcrypto
016_LDFLAGS= -lssl -lcrypto
017_CFLAGS= -I../include
017_LDADD= -lprintint -lssl -lcrypto
017_LDFLAGS= -L../lib
018_SOURCES= 018.c server.c
019_LDFLAGS=-lm
020_SOURCES= 020.c 020.h
022_SOURCES= 022.c 022.h
020_CFLAGS= -I../include
022_CFLAGS=-I../include -DLOCALEDIR='"$(localedir)"'
022_LDADD= -ltextdomain
022_LDFLAGS=-L../lib
026_SOURCES = 026.c 026.h
026_CFLAGS = -I../include
028_SOURCES = 028.c sha-1.c sha-2.c sha-3.c sha-n.c main028.h sha-1.h sha-2.h sha-3.h sha-n.h 028.h
028_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
bm_SOURCES = bitmessage.c base64.c api.c xmlrpc.c bm.c bitmessage.h base64.h api.h xmlrpc.h
bm_CFLAGS = `xmlrpc-c-config libwww-client --cflags`
bm_LDFLAGS = `xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
cudahello_SOURCES=cudahello.cu
cudahello_CFLAGS=-fPIC
rsa1024_SOURCES=RSA-1024-challenge.c

