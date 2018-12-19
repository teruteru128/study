bin_PROGRAMS=bm

bm_SOURCES= bitmessage.c base64.c api.c xmlrpc.c bm.c

CFLAGS += `xmlrpc-c-config libwww-client --cflags`
CLIBS = `xmlrpc-c-config libwww-client --libs`

