SUBDIRS = config

bin_PROGRAMS= \
	CoDec \
	FizzBuzz \
	Nabeatsu \
	Nabeatsu2 \
	Nabeatsu3 \
	TSIdentityTool \
	addpadding \
	ankokudan_decode \
	argsample \
	argparsesample \
	bcasttest \
	bino \
	bm \
	bmkeygen \
	bmkeysearch \
	bmkeysearch2 \
	bmspam \
	bouyomi \
	calc \
	cl \
	cp2peq \
	curlonionheadertest \
	curlonionheadertest2 \
	curloniontest \
	curltest \
	dappun \
	dice \
	dicetohex \
	doublerandom \
	dragon \
	dragon \
	eckeygentest \
	encodersa \
	enginetest \
	envgettest \
	errnosample1 \
	errnosample2 \
	evpmdtest \
	evprsasignsample \
	evprsaverifysample \
	evptest \
	falcon \
	ftm \
	ftnr \
	genericsample \
	getnyanpass \
	getnyanpass2 \
	gettextsample2 \
	gettextsample3 \
	gettextsample4 \
	gettime \
	harmonic \
	httpd \
	hugecollatztest \
	iconvtest \
	itoabench \
	json-c_test \
	kxsample \
	list2 \
	localesample1 \
	localesample2 \
	localesample3 \
	log-sample \
	loop-bench \
	main \
	mappingsample \
	maxmovingsum \
	md5fullzero \
	mineseed \
	mysqlaccesssample \
	mysqlaccesssample2 \
	mysqlfast \
	pgpcrc24sample \
	pkeyencsample \
	portscanner \
	potest \
	prime1 \
	prime2 \
	prime3 \
	prime4 \
	prime5 \
	prime6 \
	prime7 \
	prime8 \
	prime9 \
	prime10 \
	prime11 \
	prime12 \
	prime13 \
	prime14 \
	primesearch \
	printBench \
	printinttest \
	randomcolor \
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
	slimesearch \
	sntpc \
	ssh-keyconvert \
	strspntest \
	study \
	sum \
	teamspeakIdUpgrader \
	teamspeakcalc \
	threadAttr \
	threadcount \
	timerfdsample \
	timerfdsample2 \
	timesample \
	totp \
	tsidbench \
	tsidimprover \
	tsidshowsample \
	tweet \
	udpreciever \
	udpsender \
	upnp-port-fowarding-test \
	uuidtest \
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

LIBSTUDYPATH=$(top_builddir)/libstudy/src/libstudy.la
CoDec_LDADD=$(LIBSTUDYPATH)
CoDec_SOURCES=CoDec.c CoDec.h
FizzBuzz_CFLAGS=
FizzBuzz_LDADD=$(LIBSTUDYPATH)
FizzBuzz_SOURCES=FizzBuzz.c
Nabeatsu_CFLAGS=
Nabeatsu_LDADD=$(LIBSTUDYPATH)
Nabeatsu_SOURCES=Nabeatsu.c gettext.h
Nabeatsu2_SOURCES=Nabeatsu2.c gettext.h
Nabeatsu3_SOURCES=Nabeatsu3.c gettext.h
TSIdentityTool_LDADD=-ltommath -ltomcrypt
ankokudan_decode_SOURCES=ankokudan_decode.c
argsample_SOURCES=argsample.c
argsample_LDADD=$(LIBSTUDYPATH)
argparsesample_SOURCES=argparsesample.c
bcasttest_SOURCES=bcasttest.c
bino_SOURCES=bino.c
bm_SOURCES=bm.c
bmkeygen_CFLAGS=
bmkeygen_LDADD=
bmkeygen_SOURCES=bmkeygen.c
bmkeysearch_CFLAGS=
bmkeysearch_LDADD=$(LIBSTUDYPATH)
bmkeysearch_SOURCES=bmkeysearch.c queue.c queue.h
bmkeysearch2_LDADD=$(LIBSTUDYPATH)
bmkeysearch2_SOURCES=bmkeysearch2.c queue.c queue.h
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
cp2peq_SOURCES=cp2peq.c
cp2peq_LDADD=$(LIBSTUDYPATH)
dice_LDADD=
dragon_LDADD=$(LIBSTUDYPATH)
dragon_SOURCES=dragon.c
eckeygentest_CFLAGS=
eckeygentest_SOURCES=eckeygentest.c gettext.h timeutil.c timeutil.h
encodersa_CFLAGS=
encodersa_SOURCES=encodersa.c gettext.h
falcon_LDADD=$(LIBSTUDYPATH)
falcon_SOURCES=falcon.c
ftnr_LDADD=$(LIBSTUDYPATH)
gettextsample2_CFLAGS=
gettextsample2_SOURCES=gettextsample2.c gettextsample2.h gettext.h
gettextsample3_CFLAGS=
gettextsample3_SOURCES=gettextsample3.c gettextsample3.h textdomain.c textdomain_inner.h mytextdomain.h gettext.h
gettextsample4_SOURCES=gettextsample4.c gettextsample4.h textdomain.c textdomain_inner.h mytextdomain.h gettext.h
gettime_LDADD=$(LIBSTUDYPATH)
gettime_SOURCES=gettime.c gettime.h
httpd_LDADD=$(LIBSTUDYPATH)
iconvtest_SOURCES=iconvtest.c
itoabench_CFLAGS=
itoabench_LDADD=$(LIBSTUDYPATH)
itoabench_SOURCES=itoabench.c
localesample1_CFLAGS=
localesample1_SOURCES=localesample1.c localesample1.h
localesample2_SOURCES=localesample2.c
localesample3_SOURCES=localesample3.c
main_SOURCES=main.cpp
mappingsample_SOURCES=mappingsample.c
maxmovingsum_SOURCES=maxmovingsum.c
maxmovingsum_LDADD=$(LIBSTUDYPATH)
md5fullzero_SOURCES=md5fullzero.c
md5fullzero_LDADD=$(LIBSTUDYPATH)
mineseed_LDADD=$(LIBSTUDYPATH)
mysqlaccesssample_SOURCES=mysqlaccesssample.c
mysqlaccesssample2_SOURCES=mysqlaccesssample2.c
portscanner_SOURCES=portscanner.c
prime1_SOURCES=prime1.c
prime2_SOURCES=prime2.c
prime3_SOURCES=prime3.c
prime6_SOURCES=prime6.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
prime7_SOURCES=prime7.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
prime8_SOURCES=prime8.c bitsieve.c bitsieve.h task_queue.c task_queue.h queue.c queue.h timeutil.c timeutil.h bitset.h
#prime8_LDFLAGS=-static
prime12_SOURCES=prime12.c bitsieve.c bitsieve.h timeutil.c timeutil.h
prime13_SOURCES=prime13.c bitsieve.c bitsieve.h queue.c queue.h timeutil.c timeutil.h bitset.h
prime14_SOURCES=prime14.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
primesearch_SOURCES=primesearch.c
printBench_CFLAGS=
printBench_LDADD=$(LIBSTUDYPATH)
printBench_SOURCES=printBench.c timeutil.c timeutil.h
printinttest_CFLAGS=
printinttest_LDADD=$(LIBSTUDYPATH)
printinttest_SOURCES=printinttest.c
read_random_LDADD=$(LIBSTUDYPATH)
read_random_SOURCES=read_random.c
rsa1024_SOURCES=rsa1024.c
selfmadelibsha_CPPFLAGS=
selfmadelibsha_LDADD=$(LIBSTUDYPATH)
selfmadelibsha_SOURCES=self-made-libsha.c self-made-libsha.h
seq_CFLAGS=
seq_SOURCES=seq.c
service_sample_SOURCES=service-sample.c server.c server.h
slimesearch_SOURCES=slimesearch.c timeutil.c timeutil.h
slimesearch_LDADD=$(LIBSTUDYPATH)
sntpc_CFLAGS=
sntpc_LDADD=$(LIBSTUDYPATH)
sntpc_SOURCES=sntpc.c ntp.c ntp.h
study_CFLAGS=
study_LDADD=$(LIBSTUDYPATH)
study_SOURCES=study.c
sum_LDADD=$(LIBSTUDYPATH)
sum_SOURCES=sum.c
teamspeakIdUpgrader_CFLAGS=
teamspeakIdUpgrader_LDADD=$(LIBSTUDYPATH)
teamspeakIdUpgrader_SOURCES=teamspeakIdUpgrader.c
teamspeakcalc_CFLAGS=
teamspeakcalc_LDADD=$(LIBSTUDYPATH)
teamspeakcalc_SOURCES=teamspeakcalc.c
threadcount_SOURCES=threadcount.c
totp_LDADD=$(LIBSTUDYPATH)
tsidimprover_LDADD=$(LIBSTUDYPATH)
tsidimprover_SOURCES=tsidimprover.c
tsidshowsample_SOURCES=tsidshowsample.c
tsidshowsample_LDADD=-ltommath -ltomcrypt
udpsender_SOURCES=udpsender.c
yattaze_SOURCES=yattaze.c yattaze.h
