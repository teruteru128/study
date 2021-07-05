SUBDIRS = config

bin_PROGRAMS= \
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
	main2 \
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

EXTRA_DIST=gettextsample.c gettext.h

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
httpd_LDADD=$(LIBSTUDYPATH)
iconvtest_SOURCES=iconvtest.c
itoabench_CFLAGS=
itoabench_LDADD=$(LIBSTUDYPATH)
itoabench_SOURCES=itoabench.c gettext.h
localesample1_CFLAGS=
localesample1_SOURCES=localesample1.c localesample1.h  gettext.h
localesample2_SOURCES=localesample2.c 
localesample3_SOURCES=localesample3.c gettext.h
main_SOURCES=main.cpp
main2_CFLAGS=
main2_LDADD=$(LIBSTUDYPATH)
main2_SOURCES=main2.c
mappingsample_SOURCES=mappingsample.c
maxmovingsum_SOURCES=maxmovingsum.c
maxmovingsum_LDADD=$(LIBSTUDYPATH)
md5fullzero_SOURCES=md5fullzero.c
md5fullzero_LDADD=$(LIBSTUDYPATH)
mineseed_LDADD=$(LIBSTUDYPATH)
mysqlaccesssample_SOURCES=mysqlaccesssample.c
mysqlaccesssample2_SOURCES=mysqlaccesssample2.c
portscanner_SOURCES=portscanner.c
potest_SOURCES=potest.c gettext.h
prime1_SOURCES=prime1.c
prime2_SOURCES=prime2.c
prime3_SOURCES=prime3.c
prime6_SOURCES=prime6.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
prime7_SOURCES=prime7.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
prime8_SOURCES=prime8.c bitsieve.c bitsieve.h task_queue.c task_queue.h queue.c queue.h timeutil.c timeutil.h bitset.h gettext.h
#prime8_LDFLAGS=-static
prime12_SOURCES=prime12.c bitsieve.c bitsieve.h timeutil.c timeutil.h
prime13_SOURCES=prime13.c bitsieve.c bitsieve.h queue.c queue.h timeutil.c timeutil.h bitset.h
prime14_SOURCES=prime14.c bitsieve.c bitsieve.h timeutil.c timeutil.h bitset.h
primesearch_SOURCES=primesearch.c
printBench_CFLAGS=
printBench_LDADD=$(LIBSTUDYPATH)
printBench_SOURCES=printBench.c timeutil.c timeutil.h gettext.h
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
sntpc_SOURCES=sntpc.c ntp.c ntp.h gettext.h
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
