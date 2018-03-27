CC=gcc
CFLAGS= -Wall -I../include -g3 -O0
LDFLAGS= -L../lib -L.
LDLIBS= -lssl -lcrypto -lprintint

all: main

main:

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
