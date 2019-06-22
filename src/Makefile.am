bin_PROGRAMS=study
bin_PROGRAMS+=teamspeakcalc
bin_PROGRAMS+=loop-bench
bin_PROGRAMS+=main
bin_PROGRAMS+=printBench
bin_PROGRAMS+=signalsample
bin_PROGRAMS+=signalsample2
bin_PROGRAMS+=teamspeakIdUpgrater
bin_PROGRAMS+=argparsesample
bin_PROGRAMS+=envgettest
bin_PROGRAMS+=uuidtest
bin_PROGRAMS+=013
bin_PROGRAMS+=014
bin_PROGRAMS+=015
bin_PROGRAMS+=016
bin_PROGRAMS+=017
bin_PROGRAMS+=018
bin_PROGRAMS+=019
bin_PROGRAMS+=gettextsample2
bin_PROGRAMS+=lzhout
bin_PROGRAMS+=gettextsample3
bin_PROGRAMS+=orz
bin_PROGRAMS+=gettime
bin_PROGRAMS+=dappun
bin_PROGRAMS+=localetest1
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
bin_PROGRAMS+=firstThread4
bin_PROGRAMS+=variables
bin_PROGRAMS+=variables3
bin_PROGRAMS+=seq
bin_PROGRAMS+=sum
bin_PROGRAMS+=rsa1024
bin_PROGRAMS+=curltest
bin_PROGRAMS+=rsa
bin_PROGRAMS+=prime3
bin_PROGRAMS+=maxmovingsum


cudahello:

%:%.cu
	nvcc -ccbin clang-4.0 $^ -o $@

#AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
study_CFLAGS= -I../include -DLOCALEDIR='"$(localedir)"'
#include_HEADERS= printint.h
study_LDADD= -lprintint -lcrypto
study_LDFLAGS= -L../lib
study_SOURCES=study.c\
							gettextsample.c\
							gettextsample.h
teamspeakcalc_CFLAGS= -I../include
teamspeakcalc_LDADD= -lprintint -lcrypto
teamspeakcalc_LDFLAGS= -L../lib
printBench_CFLAGS= -I../include
printBench_LDADD= -lprintint -lssl -lcrypto
printBench_LDFLAGS= -L../lib
main_SOURCES=main.c base64.c
teamspeakIdUpgrater_SOURCES=teamspeakIdUpgrater.c
teamspeakIdUpgrater_CFLAGS= -I../include
teamspeakIdUpgrater_LDADD= -lprintint -lssl -lcrypto
teamspeakIdUpgrater_LDFLAGS= -L../lib
argparsesample_SOURCES=argparsesample.c
uuidtest_LDFLAGS= -luuid
015_LDFLAGS= -lcrypto
016_LDFLAGS= -lssl -lcrypto
017_CFLAGS= -I../include
017_LDADD= -lprintint -lssl -lcrypto
017_LDFLAGS= -L../lib
018_SOURCES= 018.c server.c
019_LDFLAGS=-lm
gettextsample2_SOURCES= gettextsample2.c gettextsample2.h
gettextsample2_CFLAGS= -I../include -DLOCALEDIR='"$(localedir)"'
gettextsample3_SOURCES= gettextsample3.c gettextsample3.h
gettextsample3_CFLAGS=-I../include -DLOCALEDIR='"$(localedir)"'
gettextsample3_LDADD= -ltextdomain
gettextsample3_LDFLAGS=-L../lib
localetest1_SOURCES = localetest1.c localetest1.h
localetest1_CFLAGS = -I../include -DLOCALEDIR='"$(localedir)"'
028_SOURCES = 028.c sha-1.c sha-2.c sha-3.c sha-n.c main028.h sha-1.h sha-2.h sha-3.h sha-n.h 028.h
028_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
bm_SOURCES = bitmessage.c base64.c bmapi.c xmlrpc.c bm.c bitmessage.h base64.h bmapi.h xmlrpc.h
bm_CFLAGS = `xmlrpc-c-config libwww-client --cflags`
bm_LDFLAGS = `xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
firstThread_LDFLAGS=-lpthread
firstThread2_LDFLAGS=-lpthread
firstThread3_LDFLAGS=-lpthread
firstThread4_LDFLAGS=-lpthread
variables_LDFLAGS=-lpthread
variables3_LDFLAGS=-lpthread
seq_CFLAGS=-lm
seq_LDFLAGS=-lm
sum_LDFLAGS=-lm
cudahello_SOURCES=cudahello.cu
cudahello_CFLAGS=-fPIC
rsa1024_SOURCES=RSA-1024-challenge.c
curltest_LDFLAGS=-lcurl

