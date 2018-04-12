
CFLAGS = -O0  -I../include
LDFLAGS = -L../lib
LDLIBS = -lcrypto

all: main

main: main.o ../lib/printint.o

.PHONY: clean

clean:
	$(RM) *.exe *.o
