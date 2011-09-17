lib_LTLIBRARIES = libm2handler.la
noinst_LTLIBRARIES = libbstr.la
noinst_bindir = `pwd`/example/bin
noinst_bin_PROGRAMS = body_toupper daemon_to_upper fifo_reader ws_handshake

libm2handler_la_LDFLAGS         = -lzmq -ljansson
libm2handler_la_LIBADD          = libbstr.la
libm2handler_la_includedir      = $(includedir)/m2handler
libm2handler_la_include_HEADERS = src/handler.h \
                                  src/websocket.h

libm2handler_la_SOURCES = src/handler.c \
                          src/websocket.c \
                          src/md5/md5.c

libbstr_la_includedir      = $(includedir)/m2handler/bstr
libbstr_la_include_HEADERS = src/bstr/bstrlib.h \
                             src/bstr/bstraux.h
libbstr_la_SOURCES         = src/bstr/bstrlib.c \
                             src/bstr/bstraux.c

handlerdir = example/handlers
body_toupper_CPPFLAGS     = -Isrc/
body_toupper_DEPENDENCIES = libm2handler.la
body_toupper_LDADD        = -lm2handler
body_toupper_SOURCES      = $(handlerdir)/body_toupper.c

daemon_to_upper_CPPFLAGS  = -Isrc/
daemon_to_upper_DEPENDENCIES = libm2handler.la
daemon_to_upper_LDADD     = -lm2handler
daemon_to_upper_SOURCES   = $(handlerdir)/daemon_to_upper.c

fifo_reader_CPPFLAGS      = -Isrc/
fifo_reader_DEPENDENCIES  = libm2handler.la
fifo_reader_LDADD         = -lm2handler
fifo_reader_SOURCES       = $(handlerdir)/fifo_reader.c

ws_handshake_CPPFLAGS     = -Isrc/
ws_handshake_DEPENDENCIES = libm2handler.la
ws_handshake_LDADD        = -lm2handler
ws_handshake_SOURCES      = $(handlerdir)/ws_handshake.c