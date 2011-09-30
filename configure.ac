AC_INIT(libm2handler, 0.5.2)

AC_PREREQ(2.61)
AM_INIT_AUTOMAKE

AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR(m4)

AC_PROG_CC_C99
AM_PROG_CC_C_O

AC_PROG_CXX

# TODO, check for zmq
# TODO, check for jansson

export CFLAGS="${CFLAGS} -L/opt/local/lib -L/usr/local/lib"
export CFLAGS="${CFLAGS} -I/opt/local/include"

AC_OUTPUT(Makefile)
