CC = gcc
CFLAGS = -Wall -I../include -fPIC -O3
LDFLAGS =
LDLIBS =

all: libsafe.so libprintint.so safe.dll printint.dll safe.lib printint.lib

test: printinttest

printinttest: printinttest.c printint.c

lib%.so: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -shared -o $@

%.dll: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -shared -o $@

%.lib: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -shared -o $@

libsafe.so:

libprintint.so:

safe.lib:

printint.lib:

safe.dll:

printint.dll:

printint.o: printint.c

safe.o: safe.c

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe *.dll *.lib
