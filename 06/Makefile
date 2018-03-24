CC = gcc
CFLAGS = -Wall
LDLIBS = -lssl -lcrypto -lnghttp2

all: main

main: main.o

main.o: main.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
