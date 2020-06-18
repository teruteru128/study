bin_PROGRAMS=
bin_PROGRAMS+=CoDec
bin_PROGRAMS+=FizzBuzz
bin_PROGRAMS+=Nabeatsu
bin_PROGRAMS+=ankokudan_decode
bin_PROGRAMS+=argparsesample
bin_PROGRAMS+=bcasttest
bin_PROGRAMS+=bmkeygen
bin_PROGRAMS+=bmspam
bin_PROGRAMS+=bouyomi
bin_PROGRAMS+=calc
bin_PROGRAMS+=cl
#bin_PROGRAMS+=cudahello
#bin_PROGRAMS+=cudamalloctest
bin_PROGRAMS+=curltest
bin_PROGRAMS+=dappun
bin_PROGRAMS+=doublerandom
bin_PROGRAMS+=dragon
bin_PROGRAMS+=dragon
bin_PROGRAMS+=eckeygentest
bin_PROGRAMS+=encodersa
bin_PROGRAMS+=enginetest
bin_PROGRAMS+=envgettest
bin_PROGRAMS+=epsp-parser-test
bin_PROGRAMS+=evptest
bin_PROGRAMS+=falcon
bin_PROGRAMS+=firstThread
bin_PROGRAMS+=firstThread2
bin_PROGRAMS+=firstThread3
bin_PROGRAMS+=firstThread4
bin_PROGRAMS+=firstThread4-1
bin_PROGRAMS+=gettextsample2
bin_PROGRAMS+=gettextsample3
bin_PROGRAMS+=gettime
bin_PROGRAMS+=hugecollatztest
bin_PROGRAMS+=itoabench
bin_PROGRAMS+=kxsample
bin_PROGRAMS+=list2
bin_PROGRAMS+=localetest1
bin_PROGRAMS+=localetest2
bin_PROGRAMS+=log-sample
bin_PROGRAMS+=loop-bench
bin_PROGRAMS+=mappingsample
bin_PROGRAMS+=maxmovingsum
bin_PROGRAMS+=mysqlfast
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
bin_PROGRAMS+=tsidimprover
bin_PROGRAMS+=tsidshowsample
bin_PROGRAMS+=tweet
bin_PROGRAMS+=udpreciever
bin_PROGRAMS+=udpsender
bin_PROGRAMS+=uuidtest
bin_PROGRAMS+=variables
bin_PROGRAMS+=variables3
bin_PROGRAMS+=variables4
bin_PROGRAMS+=variables5
bin_PROGRAMS+=yattaze

# TODO: *.cuをコンパイルするときだけNVCCを使いたい
# https://tschoonj.github.io/blog/2014/05/10/building-a-cuda-device-function-library-with-autotools/
# :thinking:?
NVCC=nvcc

#.cu.o:
#	$(NVCC) -ccbin $(CC) -c $< -o $@

#cudahello: cudahello.o
#	$(NVCC) $(cudahello_LDFLAGS) -ccbin $(CC) $< -o $@

#cudamalloctest: cudamalloctest.o
#	$(NVCC) $(cudamalloctest_LDFLAGS) -ccbin $(CC) $< -o $@

AM_CPPFLAGS=-Wall
CoDec_SOURCES=CoDec.c charset-convert.c charset-convert.h config.h CoDec.h
FizzBuzz_CFLAGS=-DLOCALEDIR='"$(localedir)"'
FizzBuzz_SOURCES=FizzBuzz.c printint.c printint.h config.h
Nabeatsu_CFLAGS=-DLOCALEDIR='"$(localedir)"'
Nabeatsu_SOURCES=Nabeatsu.c printint.c printint.h study-config.h config.h
ankokudan_decode_SOURCES=ankokudan_decode.c
argparsesample_SOURCES=argparsesample.c
bcasttest_SOURCES=bcasttest.c config.h
bmkeygen_SOURCES=bmkeygen.c study-config.h config.h random.h random.c numsys.c numsys.h bm.h bm.c
bmkeygen_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bmspam_CFLAGS=`xmlrpc-c-config libwww-client --cflags`
bmspam_LDFLAGS=`xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
bmspam_SOURCES=bitmessage.c base64.c base64.h bmapi.c bmapi.h xmlrpc.c xmlrpc.h bm.c bm.h bitmessage.h config.h bmspam.c
bouyomi_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bouyomi_LDFLAGS=
bouyomi_SOURCES=print_addrinfo.c print_addrinfo.h charset-convert.c charset-convert.h bouyomi.c bouyomi.h study-config.h config.h
calc_SOURCES=calc.c config.h
cl_SOURCES=cl.c config.h print_addrinfo.c print_addrinfo.h config.h
#cudahello_LINK=$(NVCC)
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
gettextsample2_CFLAGS=-DLOCALEDIR='"$(localedir)"'
gettextsample2_SOURCES=gettextsample2.c gettextsample2.h study-config.h config.h
gettextsample3_CFLAGS=-DLOCALEDIR='"$(localedir)"'
gettextsample3_LDFLAGS=
gettextsample3_SOURCES=gettextsample3.c gettextsample3.h study-config.h config.h textdomain.c textdomain_inner.h mytextdomain.h
gettime_SOURCES=gettime.c gettime.h
hugecollatztest_LDFLAGS=-lcrypto
itoabench_SOURCES=printint.c itoabench.c printint.h config.h
itoabench_CFLAGS=-DLOCALEDIR='"$(localedir)"'
kxsample_LDFLAGS=-lsodium
localetest1_CFLAGS=-DLOCALEDIR='"$(localedir)"'
localetest1_SOURCES=localetest1.c localetest1.h
localetest2_SOURCES=localetest2.c
mappingsample_SOURCES=mappingsample.c config.h
mappingsample_LDFLAGS=-lm
maxmovingsum_LDFLAGS=-lm
maxmovingsum_SOURCES=maxmovingsum.c config.h
portscanner_SOURCES=portscanner.c config.h
prime2_SOURCES=prime2.c config.h
prime3_SOURCES=prime3.c config.h
prime_SOURCES=prime.c config.h
primesearch_SOURCES=primesearch.c config.h
printBench_CFLAGS=-DLOCALEDIR='"$(localedir)"'
printBench_LDFLAGS=-lssl -lcrypto
printBench_SOURCES=printBench.c printint.c printint.h config.h
read_random_SOURCES=read_random.c random.c random.h internal_random.h base64.c base64.h
rsa1024_SOURCES=rsa1024.c config.h
selfmadelibsha_CPPFLAGS=-DLOCALEDIR='"$(localedir)"'
selfmadelibsha_SOURCES=self-made-libsha.c sha-1.c sha-1.h sha-2.c sha-2.h sha-3.c sha-3.h self-made-libsha.h
seq_CFLAGS=
seq_LDFLAGS=-lm
seq_SOURCES=seq.c config.h
service_sample_SOURCES=service-sample.c server.c server.h
sntpc_CFLAGS=-DLOCALEDIR='"$(localedir)"'
sntpc_SOURCES=sntpc.c random.c random.h internal_random.h print_addrinfo.c print_addrinfo.h study-config.h config.h
string_list_test_SOURCES=string_list.c string_list_test.c
study_CFLAGS=-DLOCALEDIR='"$(localedir)"'
study_LDFLAGS=-lcrypto
study_SOURCES=study.c gettextsample.c gettextsample.h printint.c printint.h random.c random.h internal_random.h bitset.c bitset.h study-config.h config.h orz.c orz.h
sum_LDFLAGS=-lm
sum_SOURCES=sum.c random.c random.h internal_random.h config.h
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDFLAGS=-lssl -lcrypto
teamspeakIdUpgrader_SOURCES=teamspeakIdUpgrader.c printint.c printint.h  config.h
teamspeakcalc_CFLAGS=
teamspeakcalc_LDFLAGS=-lcrypto
teamspeakcalc_SOURCES=teamspeakcalc.c printint.c printint.h config.h
tsidimprover_LDFLAGS=-lssl -lcrypto
tsidimprover_SOURCES=tsidimprover.c printint.c printint.h config.h random.h random.c numsys.c numsys.h
tsidshowsample_LDFLAGS=-lssl -lcrypto
tsidshowsample_SOURCES=tsidshowsample.c config.h
threadAttr_LDFLAGS=-pthread
udpsender_SOURCES=udpsender.c config.h
uuidtest_LDFLAGS=-luuid
variables3_LDFLAGS=-lpthread
variables_LDFLAGS=-lpthread
