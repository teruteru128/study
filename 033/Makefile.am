bin_PROGRAMS=bm

bm_SOURCES= bitmessage.c base64.c api.c xmlrpc.c bm.c

bm_CFLAGS += `xmlrpc-c-config libwww-client --cflags`
bm_LDFLAGS = `xmlrpc-c-config libwww-client --ldflags` `xmlrpc-c-config libwww-client --libs`

