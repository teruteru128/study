CC = gcc
CFLAGS = -Wall -I../include
LDFLAGS = -L/usr/local/lib -L/cygdrive/c/Users/onose/git/study/lib
LDLIBS = -lssl -lcrypto -lsafe

all: main

main: main.o

main.o: main.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
