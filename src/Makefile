OPTCFLAGS=-I/opt/local/include
OPTLFLAGS=-L/opt/local/lib
LOCAL_MONGREL2_LIB=-L. -lm2handler

CFLAGS=-g -std=gnu99 -Wall -Werror -I. -Imd5/ -Ibstr/ -I/opt/local/include
LFLAGS=-L/opt/local/lib $(OPTLFLAGS)
LIBS=-lzmq -ljansson
SRCS=$(wildcard *.c bstr/*.c md5/*.c)
OBJS=$(patsubst %.c,%.o,${SRCS})

# # uncomment me to print raw debug information
# CFLAGS+=-DDEBUG

SAMPLE_SRCS=$(wildcard sample_handlers/*.c)
SAMPLE_OBJS=$(patsubst %.c,%.o,${SAMPLE_SRCS})
SAMPLE_APPS=$(patsubst %.c,%,${SAMPLE_SRCS})

PREFIX?=/usr/local

SOURCES=m2handler.c bstr/bstrlib.c bstrlib.c

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

libm2handler.a: $(OBJS)
	ar rcs libm2handler.a $(OBJS)
	ranlib libm2handler.a

$(SAMPLE_APPS):$(SAMPLE_OBJS)
	$(CC) $@.o -o $@ -L. -lm2handler -ljansson -lzmq -Ibstr/ -I/usr/local/include -I.

test: libm2handler.a $(SAMPLE_APPS)

scan-build:
	scan-build make test

install: all
	install -d -o root $(PREFIX)/lib/
	install -o root libm2handler.a $(PREFIX)/lib/
	install -d -o root $(PREFIX)/include/
	install -o root m2handler.h $(PREFIX)/include/
	install -o root m2websocket.h $(PREFIX)/include/
	install -d -o root $(PREFIX)/include/bstr/
	install -o root bstr/bstrlib.h $(PREFIX)/include/bstr/
	install -o root bstr/bstraux.h $(PREFIX)/include/bstr/

clean : 
	rm -f $(OBJS) $(SAMPLE_APPS) libm2handler.a handler_pipe

all: libm2handler.a
