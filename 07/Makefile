CC = gcc
CFLAGS = -Wall
LDLIBS = -lssl -lcrypto -loauth -lnghttp2 -lcurl

all: tw_console tw_post

tw_console: tw_console.o

tw_console.o: tw_console.c

tw_post: tw_post.o

.PHONY: clean

clean:
	$(RM) -f lib*.so *.o *.exe
