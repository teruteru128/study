bin_PROGRAMS=study
bin_PROGRAMS+=teamspeakcalc
bin_PROGRAMS+=loop-bench
bin_PROGRAMS+=read_random
bin_PROGRAMS+=printBench
bin_PROGRAMS+=signalsample
bin_PROGRAMS+=signalsample2
bin_PROGRAMS+=teamspeakIdUpgrader
bin_PROGRAMS+=argparsesample
bin_PROGRAMS+=envgettest
bin_PROGRAMS+=uuidtest
bin_PROGRAMS+=013
bin_PROGRAMS+=014
bin_PROGRAMS+=015
bin_PROGRAMS+=016
bin_PROGRAMS+=017
bin_PROGRAMS+=service-sample
bin_PROGRAMS+=019
bin_PROGRAMS+=gettextsample2
bin_PROGRAMS+=lzhout
bin_PROGRAMS+=gettextsample3
bin_PROGRAMS+=orz
bin_PROGRAMS+=gettime
bin_PROGRAMS+=dappun
bin_PROGRAMS+=localetest1
bin_PROGRAMS+=selfmadelibsha
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
bin_PROGRAMS+=list2
bin_PROGRAMS+=threadAttr
bin_PROGRAMS+=yattaze
bin_PROGRAMS+=encodersa
bin_PROGRAMS+=CoDec
bin_PROGRAMS+=iconv_Chk
bin_PROGRAMS+=falcon
bin_PROGRAMS+=dragon
bin_PROGRAMS+=doublerandom
bin_PROGRAMS+=cudahello
bin_PROGRAMS+=FizzBuzz
bin_PROGRAMS+=Nabeatsu
bin_PROGRAMS+=epsp-parser-test
bin_PROGRAMS+=sntpc
bin_PROGRAMS+=string_list_test


%:%.cu
	nvcc -ccbin clang-4.0 $^ -o $@

#AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
study_CFLAGS= -DLOCALEDIR='"$(localedir)"'
study_LDADD= -lcrypto
study_LDFLAGS=
study_SOURCES=study.c\
							gettextsample.c\
							gettextsample.h\
							printint.c\
							random.c\
							random.h
teamspeakcalc_CFLAGS=
teamspeakcalc_LDADD= -lcrypto
teamspeakcalc_LDFLAGS=
teamspeakcalc_SOURCES= teamspeakcalc.c printint.c
printBench_CFLAGS=
printBench_LDADD= -lssl -lcrypto
printBench_LDFLAGS=
printBench_SOURCES= printBench.c printint.c
read_random_SOURCES=read_random.c base64.c
teamspeakIdUpgrader_SOURCES= teamspeakIdUpgrader.c printint.c
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDADD= -lssl -lcrypto
teamspeakIdUpgrader_LDFLAGS=
argparsesample_SOURCES=argparsesample.c
uuidtest_LDFLAGS= -luuid
013_LDFLAGS= -lsodium
015_LDFLAGS= -lcrypto
016_LDFLAGS= -lssl -lcrypto
017_CFLAGS=
017_LDADD= -lssl -lcrypto
017_LDFLAGS=
017_SOURCES= 017.c printint.c
service_sample_SOURCES= service-sample.c server.c
019_LDFLAGS=-lm
gettextsample2_SOURCES= gettextsample2.c gettextsample2.h
gettextsample2_CFLAGS= -DLOCALEDIR='"$(localedir)"'
gettextsample3_SOURCES= gettextsample3.c gettextsample3.h textdomain.c
gettextsample3_CFLAGS= -DLOCALEDIR='"$(localedir)"'
gettextsample3_LDADD=
gettextsample3_LDFLAGS=
orz_SOURCES= orz.c random.h random.c
localetest1_SOURCES = localetest1.c localetest1.h
#localetest1_CFLAGS = -I../include -DLOCALEDIR='"$(localedir)"'
localetest1_CFLAGS = -DLOCALEDIR='"$(localedir)"'
selfmadelibsha_SOURCES = self-made-libsha.c sha-1.c sha-2.c sha-3.c sha-n.c sha-1.h sha-2.h sha-3.h sha-n.h 028.h
selfmadelibsha_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
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
threadAttr_LDFLAGS=-pthread
encodersa_LDFLAGS=-lcrypto -lssl
falcon_SOURCES=falcon.c random.c random.h
dragon_SOURCES=dragon.c svgplot.c svgplot.h
Nabeatsu_SOURCES=FizzBuzz.c
epsp_parser_test_SOURCES=epsp-parser-test.c epsp-parser.c epsp-parser.h
sntpc_SOURCES=sntpc.c
string_list_test_SOURCES=string_list.h string_list.c string_list_test.c
