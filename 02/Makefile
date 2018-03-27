CC = gcc
CFLAGS = -Wall -I../include -g3
LDFLAGS = -L/usr/local/lib -L../lib
LDLIBS = -lssl -lcrypto -lsafe

all: main

main: main.o

main.o: main.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
