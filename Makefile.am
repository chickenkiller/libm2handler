lib_LTLIBRARIES = libm2handler.la
noinst_LTLIBRARIES = libbstr.la libdict.la libsha1.la
noinst_bindir = `pwd`/example/bin
noinst_bin_PROGRAMS = $(sample_handlers) \
					  $(test_programs)

AM_CFLAGS=-g -std=gnu99 -Wall -Werror -Isrc/ -I/opt/local/include
AM_CXXFLAGS=-g -Wall -Werror -Isrc/ -I/opt/local/include

sample_handlers=body_toupper daemon_to_upper fifo_reader ws_handshake ws_cpp_handshake ws_variable
test_programs=ws_accept framing dict jansson

libm2handler_la_LDFLAGS         = -lzmq -ljansson
libm2handler_la_LIBADD          = libbstr.la libdict.la libsha1.la
libm2handler_la_includedir      = $(includedir)/m2handler
libm2handler_la_include_HEADERS = src/handler.h \
                                  src/websocket.h \
                                  src/websocket_framing.h \
                                  src/websocket_session.h \
                                  src/debug.h

libm2handler_la_SOURCES = src/handler.c \
                          src/websocket.c \
                          src/websocket_framing.c \
                          src/websocket_session.c \
                          src/debug.c

libsha1_la_includedir      = $(includedir)/m2handler/sha1
libsha1_la_include_HEADERS = src/sha1/sha1.h \
                             src/sha1/config.h
libsha1_la_SOURCES         = src/sha1/sha1.c

libbstr_la_includedir      = $(includedir)/m2handler/bstr
libbstr_la_include_HEADERS = src/bstr/bstrlib.h \
                             src/bstr/bstraux.h
libbstr_la_SOURCES         = src/bstr/bstrlib.c \
                             src/bstr/bstraux.c

libdict_la_includedir      = $(includedir)/src/adt
libdict_la_include_HEADERS = src/adt/dict.h
libdict_la_SOURCES         = src/adt/dict.c

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

ws_cpp_handshake_CPPFLAGS     = -Isrc/
ws_cpp_handshake_DEPENDENCIES = libm2handler.la
ws_cpp_handshake_LDADD        = -lm2handler
ws_cpp_handshake_SOURCES      = $(handlerdir)/ws_handshake.cpp

ws_variable_DEPENDENCIES  = libm2handler.la
ws_variable_LDADD         = -lm2handler
ws_variable_SOURCES       = $(handlerdir)/ws_variable.c

testdir = test
ws_accept_CPPFLAGS        = -Isrc/
ws_accept_DEPENDENCIES    = libm2handler.la
ws_accept_LDADD           = -lm2handler
ws_accept_SOURCES         = $(testdir)/ws_accept.c

framing_CPPFLAGS          = -Isrc/
framing_DEPENDENCIES      = libm2handler.la
framing_LDADD			        = -lm2handler
framing_SOURCES           = $(testdir)/framing.c

dict_CPPFLAGS             = -Isrc/
dict_DEPENDENCIES         = libm2handler.la
dict_LDADD                = -lm2handler
dict_SOURCES              = $(testdir)/dict.c

# session_CPPFLAGS          = -Isrc/
# session_DEPENDENCIES      = libm2handler.la
# session_LDADD             = -lm2handler
# session_SOURCES           = $(testdir)/session.c

jansson_LDADD             = -ljansson
jansson_SOURCES           = $(testdir)/jansson.c

test: $(test_programs)

handlers: $(sample_handlers)
