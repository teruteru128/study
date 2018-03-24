CC = gcc
CFLAGS = -Wall -I../include
LDFLAGS = -L../lib
LDLIBS = -lssl -lcrypto -lsafe

all: main

main: main.o

main.o: main.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
