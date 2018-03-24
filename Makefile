CC = gcc
CFLAGS = -Wall -I../include -fPIC
LDFLAGS = -L/usr/local/lib -L../lib -shared
LDLIBS =

all:
#	cd 02; make
	cd 03; make
#	cd 04; make
	cd 05; make
	cd 06; make
	cd lib; make

.PHONY: clean

clean:
	cd 02; make clean
	cd 03; make clean
	cd 04; make clean
	cd 05; make clean
	cd 06; make clean
	cd lib; make clean
