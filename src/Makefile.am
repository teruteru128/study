bin_PROGRAMS=
bin_PROGRAMS+=014
bin_PROGRAMS+=015
bin_PROGRAMS+=016
bin_PROGRAMS+=017
bin_PROGRAMS+=019
bin_PROGRAMS+=CoDec
bin_PROGRAMS+=FizzBuzz
bin_PROGRAMS+=Nabeatsu
bin_PROGRAMS+=ankokudan_decode
bin_PROGRAMS+=argparsesample
bin_PROGRAMS+=bcasttest
bin_PROGRAMS+=bm
bin_PROGRAMS+=bmkeygen
bin_PROGRAMS+=bouyomi
bin_PROGRAMS+=calc
bin_PROGRAMS+=cl
bin_PROGRAMS+=cudahello
bin_PROGRAMS+=cudamalloctest
bin_PROGRAMS+=curltest
bin_PROGRAMS+=dappun
bin_PROGRAMS+=doublerandom
bin_PROGRAMS+=dragon
bin_PROGRAMS+=dragon
bin_PROGRAMS+=eckeygentest
bin_PROGRAMS+=encodersa
bin_PROGRAMS+=envgettest
bin_PROGRAMS+=epsp-parser-test
bin_PROGRAMS+=falcon
bin_PROGRAMS+=firstThread
bin_PROGRAMS+=firstThread2
bin_PROGRAMS+=firstThread3
bin_PROGRAMS+=firstThread4
bin_PROGRAMS+=firstThread4-1
bin_PROGRAMS+=gettextsample2
bin_PROGRAMS+=gettextsample3
bin_PROGRAMS+=gettime
bin_PROGRAMS+=kxsample
bin_PROGRAMS+=list2
bin_PROGRAMS+=localetest1
bin_PROGRAMS+=log-sample
bin_PROGRAMS+=loop-bench
bin_PROGRAMS+=maxmovingsum
bin_PROGRAMS+=mysqlfast
bin_PROGRAMS+=orz
bin_PROGRAMS+=portscanner
bin_PROGRAMS+=prime
bin_PROGRAMS+=prime2
bin_PROGRAMS+=prime3
bin_PROGRAMS+=primesearch
bin_PROGRAMS+=printBench
bin_PROGRAMS+=read_random
bin_PROGRAMS+=repeated-trials
bin_PROGRAMS+=rsa
bin_PROGRAMS+=rsa1024
bin_PROGRAMS+=selfmadelibsha
bin_PROGRAMS+=seq
bin_PROGRAMS+=service-sample
bin_PROGRAMS+=signalsample
bin_PROGRAMS+=signalsample2
bin_PROGRAMS+=sntpc
bin_PROGRAMS+=string_list_test
bin_PROGRAMS+=study
bin_PROGRAMS+=sum
bin_PROGRAMS+=teamspeakIdUpgrader
bin_PROGRAMS+=teamspeakcalc
bin_PROGRAMS+=threadAttr
bin_PROGRAMS+=udpreciever
bin_PROGRAMS+=udpsender
bin_PROGRAMS+=uuidtest
bin_PROGRAMS+=variables
bin_PROGRAMS+=variables3
bin_PROGRAMS+=variables4
bin_PROGRAMS+=yattaze

NVCC=nvcc

.cu.o:
	$(NVCC) $(CFLAGS) -ccbin $(CC) -c $< -o $@

cudahello: cudahello.o
	$(NVCC) $(LDFLAGS) -ccbin $(CC) $< -o $@

cudamalloctest: cudamalloctest.o
	$(NVCC) $(LDFLAGS) -ccbin $(CC) $< -o $@

014_SOURCES= 014.c
015_LDFLAGS= -lcrypto
016_LDFLAGS= -lssl -lcrypto
016_SOURCES= 016.c config.h
017_LDFLAGS= -lssl -lcrypto
017_SOURCES= 017.c printint.c printint.h config.h
019_SOURCES= 019.c config.h
019_LDFLAGS=-lm
CoDec_SOURCES=CoDec.c charset-convert.c charset-convert.h config.h
FizzBuzz_CFLAGS=-DLOCALEDIR='"$(localedir)"'
FizzBuzz_SOURCES=FizzBuzz.c printint.c printint.h config.h
Nabeatsu_CFLAGS=-DLOCALEDIR='"$(localedir)"'
Nabeatsu_SOURCES=FizzBuzz.c printint.c printint.h study-config.h config.h
argparsesample_SOURCES=argparsesample.c
bcasttest_SOURCES=bcasttest.c config.h
bm_CFLAGS = `xmlrpc-c-config libwww-client --cflags`
bm_LDFLAGS = `xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
bm_SOURCES = bitmessage.c base64.c base64.h bmapi.c bmapi.h xmlrpc.c xmlrpc.h bm.c bitmessage.h config.h
bmkeygen_SOURCES=bmkeygen.c study-config.h config.h
bmkeygen_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bouyomi_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bouyomi_LDFLAGS=
bouyomi_SOURCES= print_addrinfo.c print_addrinfo.h charset-convert.c charset-convert.h bouyomi.c bouyomi.h study-config.h config.h
calc_SOURCES=calc.c config.h
cl_SOURCES=cl.c config.h print_addrinfo.c print_addrinfo.h config.h
cudahello_CC=$(NVCC)
cudahello_CFLAGS=-fPIC
cudahello_SOURCES=cudahello.cu config.h
cudamalloctest_CFLAGS=-fPIC
cudamalloctest_SOURCES=cudamalloctest.cu config.h
curltest_LDFLAGS=-lcurl
curltest_SOURCES=curltest.c config.h
dragon_SOURCES=dragon.c svgplot.c svgplot.h
eckeygentest_CFLAGS=-DLOCALEDIR='"$(localedir)"'
eckeygentest_LDFLAGS=-lcrypto -lssl
eckeygentest_SOURCES=eckeygentest.c study-config.h config.h
encodersa_CFLAGS=-DLOCALEDIR='"$(localedir)"'
encodersa_LDFLAGS=-lcrypto -lssl
encodersa_SOURCES=encodersa.c study-config.h config.h
epsp_parser_test_SOURCES=epsp-parser-test.c epsp-parser.c epsp-parser.h string_array.c string_array.h string_list.c string_list.h
falcon_SOURCES=falcon.c random.c random.h internal_random.h
firstThread2_LDFLAGS=-lpthread
firstThread3_LDFLAGS=-lpthread
firstThread4_1_LDFLAGS=-lpthread
firstThread4_LDFLAGS=-lpthread
firstThread_LDFLAGS=-lpthread
gettextsample2_CFLAGS= -DLOCALEDIR='"$(localedir)"'
gettextsample2_SOURCES= gettextsample2.c gettextsample2.h study-config.h config.h
gettextsample3_CFLAGS= -DLOCALEDIR='"$(localedir)"'
gettextsample3_LDFLAGS=
gettextsample3_SOURCES= gettextsample3.c gettextsample3.h study-config.h config.h textdomain.c textdomain_inner.h mytextdomain.h
gettime_SOURCES= gettime.c gettime.h
kxsample_LDFLAGS= -lsodium
localetest1_CFLAGS = -DLOCALEDIR='"$(localedir)"'
localetest1_SOURCES = localetest1.c localetest1.h
maxmovingsum_LDFLAGS=-lm
maxmovingsum_SOURCES=maxmovingsum.c config.h
orz_SOURCES= orz.c random.c random.h internal_random.h config.h
portscanner_SOURCES=portscanner.c config.h
prime2_SOURCES=prime2.c config.h
prime3_SOURCES=prime3.c config.h
prime_SOURCES=prime.c config.h
primesearch_SOURCES=primesearch.c config.h
printBench_CFLAGS=
printBench_LDFLAGS= -lssl -lcrypto
printBench_SOURCES= printBench.c printint.c printint.h config.h
read_random_SOURCES=read_random.c random.c random.h internal_random.h base64.c base64.h
rsa1024_SOURCES=rsa1024.c config.h
selfmadelibsha_CPPFLAGS = -DLOCALEDIR='"$(localedir)"'
selfmadelibsha_SOURCES = self-made-libsha.c sha-1.c sha-1.h sha-2.c sha-2.h sha-3.c sha-3.h self-made-libsha.h
seq_CFLAGS=
seq_LDFLAGS=-lm
seq_SOURCES=seq.c config.h
service_sample_SOURCES= service-sample.c server.c server.h
sntpc_CFLAGS=-DLOCALEDIR='"$(localedir)"'
sntpc_SOURCES=sntpc.c random.c random.h internal_random.h print_addrinfo.c print_addrinfo.h study-config.h config.h
string_list_test_SOURCES=string_list.c string_list_test.c
study_CFLAGS= -DLOCALEDIR='"$(localedir)"'
study_LDFLAGS= -lcrypto
study_SOURCES=study.c gettextsample.c gettextsample.h printint.c printint.h random.c random.h internal_random.h bitset.c bitset.h study-config.h config.h gettextsample.h
sum_LDFLAGS=-lm
sum_SOURCES= sum.c random.c random.h internal_random.h config.h
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDFLAGS= -lssl -lcrypto
teamspeakIdUpgrader_SOURCES= teamspeakIdUpgrader.c printint.c printint.h  config.h
teamspeakcalc_CFLAGS=
teamspeakcalc_LDFLAGS= -lcrypto
teamspeakcalc_SOURCES= teamspeakcalc.c printint.c printint.h config.h
threadAttr_LDFLAGS=-pthread
udpsender_SOURCES=udpsender.c config.h
uuidtest_LDFLAGS= -luuid
variables3_LDFLAGS=-lpthread
variables_LDFLAGS=-lpthread
