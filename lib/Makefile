CC = gcc
CFLAGS = -Wall -I../include -fPIC
LDFLAGS =
LDLIBS =

all: libsafe.so libprintint.so

lib%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -shared -o $@

libsafe.so:

libprintint.so:

printint.o: printint.c

safe.o: safe.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
