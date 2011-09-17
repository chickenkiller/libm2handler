lib_LTLIBRARIES = libm2handler.la
noinst_LTLIBRARIES = libbstr.la

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