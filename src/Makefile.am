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
bin_PROGRAMS+=cudahello
bin_PROGRAMS+=cudamalloctest
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
bin_PROGRAMS+=primeNumber
bin_PROGRAMS+=primeNumber2
bin_PROGRAMS+=primeNumber3
bin_PROGRAMS+=primeNumber4
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
bin_PROGRAMS+=variablesProc
bin_PROGRAMS+=yattaze

# TODO: *.cuをコンパイルするときだけNVCCを使いたい
# https://tschoonj.github.io/blog/2014/05/10/building-a-cuda-device-function-library-with-autotools/
# :thinking:?
NVCC=nvcc

.cu.o:
	$(NVCC) $(DEFAULT_INCLUDES) $(INCLUDES) -I$(top_srcdir)/libstudy/include -ccbin $(CC) -c $< -o $@

cudahello: cudahello.o
	$(NVCC) $(cudahello_LDFLAGS) -ccbin $(CC) $< -o $@

cudamalloctest: cudamalloctest.o
	$(NVCC) $(cudamalloctest_LDFLAGS) -ccbin $(CC) $< -o $@

# see https://www.gnu.org/software/automake/manual/html_node/Subpackages.html
AM_CPPFLAGS=-Wall -I$(top_srcdir)/libstudy/include
CoDec_LDADD=../libstudy/src/libstudy.la
CoDec_SOURCES=CoDec.c CoDec.h
FizzBuzz_CFLAGS=-DLOCALEDIR='"$(localedir)"'
FizzBuzz_LDADD=../libstudy/src/libstudy.la
FizzBuzz_SOURCES=FizzBuzz.c
Nabeatsu_CFLAGS=-DLOCALEDIR='"$(localedir)"'
Nabeatsu_LDADD=../libstudy/src/libstudy.la
Nabeatsu_SOURCES=Nabeatsu.c study-config.h
ankokudan_decode_SOURCES=ankokudan_decode.c
argparsesample_SOURCES=argparsesample.c
bcasttest_SOURCES=bcasttest.c
bmkeygen_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bmkeygen_LDADD=../libstudy/src/libstudy.la
bmkeygen_SOURCES=bmkeygen.c study-config.h
bmspam_CFLAGS=`xmlrpc-c-config libwww-client --cflags`
bmspam_LDADD=../libstudy/src/libstudy.la
bmspam_LDFLAGS=`xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
bmspam_SOURCES=bmspam.c
bouyomi_CFLAGS=-DLOCALEDIR='"$(localedir)"'
bouyomi_LDADD=../libstudy/src/libstudy.la
bouyomi_SOURCES=bouyomi.c bouyomi.h study-config.h
calc_SOURCES=calc.c
cl_LDADD=../libstudy/src/libstudy.la
cl_SOURCES=cl.c
cudahello_CFLAGS=-fPIC
#cudahello_LINK=$(NVCC)
cudahello_SOURCES=cudahello.cu
cudamalloctest_CFLAGS=-fPIC
cudamalloctest_SOURCES=cudamalloctest.cu
curltest_SOURCES=curltest.c
dragon_LDADD=../libstudy/src/libstudy.la
dragon_SOURCES=dragon.c
eckeygentest_CFLAGS=-DLOCALEDIR='"$(localedir)"'
eckeygentest_SOURCES=eckeygentest.c study-config.h
encodersa_CFLAGS=-DLOCALEDIR='"$(localedir)"'
encodersa_SOURCES=encodersa.c study-config.h
epsp_parser_test_LDADD=../libstudy/src/libstudy.la
epsp_parser_test_SOURCES=epsp-parser-test.c
falcon_LDADD=../libstudy/src/libstudy.la
falcon_SOURCES=falcon.c
gettextsample2_CFLAGS=-DLOCALEDIR='"$(localedir)"'
gettextsample2_SOURCES=gettextsample2.c gettextsample2.h study-config.h
gettextsample3_CFLAGS=-DLOCALEDIR='"$(localedir)"'
gettextsample3_SOURCES=gettextsample3.c gettextsample3.h study-config.h textdomain.c textdomain_inner.h mytextdomain.h
gettime_SOURCES=gettime.c gettime.h
itoabench_CFLAGS=-DLOCALEDIR='"$(localedir)"'
itoabench_LDADD=../libstudy/src/libstudy.la
itoabench_SOURCES=itoabench.c
localetest1_CFLAGS=-DLOCALEDIR='"$(localedir)"'
localetest1_SOURCES=localetest1.c localetest1.h
localetest2_SOURCES=localetest2.c
mappingsample_SOURCES=mappingsample.c
maxmovingsum_SOURCES=maxmovingsum.c
portscanner_SOURCES=portscanner.c
prime2_SOURCES=prime2.c
prime3_SOURCES=prime3.c
prime_SOURCES=prime.c
primesearch_SOURCES=primesearch.c
printBench_CFLAGS=-DLOCALEDIR='"$(localedir)"'
printBench_LDADD=../libstudy/src/libstudy.la
printBench_SOURCES=printBench.c
read_random_LDADD=../libstudy/src/libstudy.la
read_random_SOURCES=read_random.c
rsa1024_SOURCES=rsa1024.c
selfmadelibsha_CPPFLAGS=-DLOCALEDIR='"$(localedir)"'
selfmadelibsha_LDADD=../libstudy/src/libstudy.la
selfmadelibsha_SOURCES=self-made-libsha.c self-made-libsha.h
seq_CFLAGS=
seq_SOURCES=seq.c
service_sample_SOURCES=service-sample.c server.c server.h
sntpc_CFLAGS=-DLOCALEDIR='"$(localedir)"'
sntpc_LDADD=../libstudy/src/libstudy.la
sntpc_SOURCES=sntpc.c study-config.h
string_list_test_LDADD=../libstudy/src/libstudy.la
string_list_test_SOURCES=string_list_test.c
study_CFLAGS=-DLOCALEDIR='"$(localedir)"'
study_LDADD=../libstudy/src/libstudy.la
study_SOURCES=study.c gettextsample.c gettextsample.h study-config.h
sum_LDADD=../libstudy/src/libstudy.la
sum_SOURCES=sum.c
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDADD=../libstudy/src/libstudy.la
teamspeakIdUpgrader_SOURCES=teamspeakIdUpgrader.c
teamspeakcalc_CFLAGS=
teamspeakcalc_LDADD=../libstudy/src/libstudy.la
teamspeakcalc_SOURCES=teamspeakcalc.c
tsidimprover_LDADD=../libstudy/src/libstudy.la
tsidimprover_SOURCES=tsidimprover.c
tsidshowsample_SOURCES=tsidshowsample.c
udpsender_SOURCES=udpsender.c
