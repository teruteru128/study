CC = gcc
CFLAGS = -Wall
#LDFLAGS = -L/usr/local/lib
LDLIBS = -lssl -loauth

all: main

main: main.o

main.o: main.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
