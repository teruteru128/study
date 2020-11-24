SUBDIRS = config

bin_PROGRAMS= \
	CoDec \
	FizzBuzz \
	Nabeatsu \
	ankokudan_decode \
	argparsesample \
	bcasttest \
	bino \
	bm \
	bmkeygen \
	bmspam \
	bouyomi \
	calc \
	cl \
	condTimedWait \
	condWait \
	cp2peq \
	curltest \
	dappun \
	dice \
	doublerandom \
	dragon \
	dragon \
	eckeygentest \
	encodersa \
	enginetest \
	envgettest \
	errnosample \
	evpmdtest \
	evptest \
	falcon \
	fastPrimeNumber \
	fastPrimeNumber2 \
	firstThread \
	firstThread2 \
	firstThread3 \
	firstThread4 \
	firstThread4-1 \
	fly \
	fly2 \
	fly2rwlock \
	fly3 \
	ftm \
	ftnr \
	gettextsample2 \
	gettextsample3 \
	gettime \
	hugecollatztest \
	iconvtest \
	itoabench \
	kxsample \
	list2 \
	localetest1 \
	localetest2 \
	log-sample \
	loop-bench \
	mappingsample \
	maxmovingsum \
	md5fullzero \
	mineseed \
	mysqlaccesssample \
	mysqlaccesssample2 \
	mysqlfast \
	pgpcrc24sample \
	portscanner \
	potest \
	prime \
	prime2 \
	prime3 \
	primeNumber \
	primeNumber2 \
	primeNumber3 \
	primeNumber4 \
	primeNumber5 \
	primesearch \
	printBench \
	read_random \
	repeated-trials \
	rsa \
	rsa1024 \
	selfmadelibsha \
	seq \
	service-sample \
	sha1rngsample \
	signalsample \
	signalsample2 \
	sntpc \
	soloFly \
	soloFly2 \
	soloFly2-2 \
	string_list_test \
	strspntest \
	study \
	sum \
	teamspeakIdUpgrader \
	teamspeakcalc \
	threadAttr \
	threadcount \
	timerfdsample \
	timesample \
	tsidimprover \
	tsidshowsample \
	tweet \
	udpreciever \
	udpsender \
	upnp-port-fowarding-test \
	uuidtest \
	variables \
	variables3 \
	variables4 \
	variables5 \
	variablesProc \
	wordconv \
	yattaze

CLEANFILES=*.aux *.dvi *.ps *.pdf

test.pdf: test.tex

.ps.pdf:
	ps2pdf $<

.dvi.ps:
	dvips $<

.tex.div:
	platex $<

EXTRA_DIST=gettextsample.c

studydatadir = $(datadir)/study

# see https://www.gnu.org/software/automake/manual/html_node/Subpackages.html
AM_CPPFLAGS=-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Winit-self -Winline -Wlogical-op -Wmissing-include-dirs -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wswitch-default -Wswitch-enum -Wunsafe-loop-optimizations -Wvariadic-macros -Wwrite-strings -I$(top_srcdir)/libstudy/include -DLOCALEDIR='"$(localedir)"' -DSTUDYDATADIR='"$(studydatadir)"'
LIBSTUDYPATH=$(top_builddir)/libstudy/src/libstudy.la
CoDec_LDADD=$(LIBSTUDYPATH)
CoDec_SOURCES=CoDec.c CoDec.h
FizzBuzz_CFLAGS=
FizzBuzz_LDADD=$(LIBSTUDYPATH)
FizzBuzz_SOURCES=FizzBuzz.c
Nabeatsu_CFLAGS=
Nabeatsu_LDADD=$(LIBSTUDYPATH)
Nabeatsu_SOURCES=Nabeatsu.c gettext.h
ankokudan_decode_SOURCES=ankokudan_decode.c
argparsesample_SOURCES=argparsesample.c
bcasttest_SOURCES=bcasttest.c
bino_SOURCES=bino.c
bm_SOURCES=bm.c
bmkeygen_CFLAGS=-fopenmp
bmkeygen_LDADD=$(LIBSTUDYPATH)
bmkeygen_SOURCES=bmkeygen.c
bmspam_CFLAGS=`xmlrpc-c-config libwww-client --cflags`
bmspam_LDADD=$(LIBSTUDYPATH)
bmspam_LDFLAGS=`xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`
bmspam_SOURCES=bmspam.c bmspam.h
bouyomi_CFLAGS=
bouyomi_LDADD=$(LIBSTUDYPATH)
bouyomi_SOURCES=bouyomi.c bouyomi.h gettext.h
calc_SOURCES=calc.c
cl_LDADD=$(LIBSTUDYPATH)
cl_SOURCES=cl.c
condTimedWait_SOURCES=condTimedWait.c
condWait_SOURCES=condWait.c
cp2peq_SOURCES=cp2peq.c
cp2peq_LDADD=$(LIBSTUDYPATH)
dice_LDADD=
dragon_LDADD=$(LIBSTUDYPATH)
dragon_SOURCES=dragon.c
eckeygentest_CFLAGS=
eckeygentest_SOURCES=eckeygentest.c gettext.h
encodersa_CFLAGS=
encodersa_SOURCES=encodersa.c gettext.h
falcon_LDADD=$(LIBSTUDYPATH)
falcon_SOURCES=falcon.c
fly_SOURCES=fly.c
fly2_SOURCES=fly2.c
fly2rwlock_SOURCES=fly2.c
ftnr_LDADD=$(LIBSTUDYPATH)
gettextsample2_CFLAGS=
gettextsample2_SOURCES=gettextsample2.c gettextsample2.h gettext.h
gettextsample3_CFLAGS=
gettextsample3_SOURCES=gettextsample3.c gettextsample3.h textdomain.c textdomain_inner.h mytextdomain.h gettext.h
gettime_LDADD=$(LIBSTUDYPATH)
gettime_SOURCES=gettime.c gettime.h
iconvtest_SOURCES=iconvtest.c
itoabench_CFLAGS=
itoabench_LDADD=$(LIBSTUDYPATH)
itoabench_SOURCES=itoabench.c
localetest1_CFLAGS=
localetest1_SOURCES=localetest1.c localetest1.h
localetest2_SOURCES=localetest2.c
mappingsample_SOURCES=mappingsample.c
maxmovingsum_SOURCES=maxmovingsum.c
maxmovingsum_LDADD=$(LIBSTUDYPATH)
mineseed_LDADD=$(LIBSTUDYPATH)
mysqlaccesssample_SOURCES=mysqlaccesssample.c
mysqlaccesssample2_SOURCES=mysqlaccesssample2.c
portscanner_SOURCES=portscanner.c
prime2_SOURCES=prime2.c
prime3_SOURCES=prime3.c
prime_SOURCES=prime.c
primesearch_SOURCES=primesearch.c
printBench_CFLAGS=
printBench_LDADD=$(LIBSTUDYPATH)
printBench_SOURCES=printBench.c
read_random_LDADD=$(LIBSTUDYPATH)
read_random_SOURCES=read_random.c
rsa1024_SOURCES=rsa1024.c
selfmadelibsha_CPPFLAGS=
selfmadelibsha_LDADD=$(LIBSTUDYPATH)
selfmadelibsha_SOURCES=self-made-libsha.c self-made-libsha.h
seq_CFLAGS=
seq_SOURCES=seq.c
service_sample_SOURCES=service-sample.c server.c server.h
sntpc_CFLAGS=
sntpc_LDADD=$(LIBSTUDYPATH)
sntpc_SOURCES=sntpc.c ntp.c ntp.h
soloFly_SOURCES=soloFly.c
soloFly2_SOURCES=soloFly2.c
soloFly2_2_SOURCES=soloFly2-2.c
string_list_test_LDADD=$(LIBSTUDYPATH)
string_list_test_SOURCES=string_list_test.c
study_CFLAGS=
study_LDADD=$(LIBSTUDYPATH)
study_SOURCES=study.c gettext.h
sum_LDADD=$(LIBSTUDYPATH)
sum_SOURCES=sum.c
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDADD=$(LIBSTUDYPATH)
teamspeakIdUpgrader_SOURCES=teamspeakIdUpgrader.c
teamspeakcalc_CFLAGS=
teamspeakcalc_LDADD=$(LIBSTUDYPATH)
teamspeakcalc_SOURCES=teamspeakcalc.c
threadcount_SOURCES=threadcount.c
tsidimprover_LDADD=$(LIBSTUDYPATH)
tsidimprover_SOURCES=tsidimprover.c
tsidshowsample_SOURCES=tsidshowsample.c
udpsender_SOURCES=udpsender.c
