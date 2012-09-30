# Mongrel2 C Handler: libm2handler

A C library for handling requests from Mongrel2. Includes a suite of sample handlers to get you up and running.

### Prequisites ###

#### mongrel2 ####

    http://www.mongrel2.org/

#### libjansson ####

    http://www.digip.org/jansson/
    https://github.com/akheron/jansson

#### GNU Automake ####
The GNU automake build system.

### Building from Scratch ###

You need to first translate the configure.ac to configure.

    libtoolize -c || glibtoolize -c
    autoreconf -fv --install

Great, now you have your build system. Now to build the Makefile.

    ./configure
    make
    sudo make install

### Valgrind ###

This software was developed with valgrind in hand.

    valgrind --leak-check=full --gen-suppressions=yes ./mongrel_handler

It throws a real shit fit about "Syscall param socketcall.sendto(msg) points to uninitialised byte(s)"
and I have no idea how to work around it. It sounds harmless.

    http://markmail.org/message/oqebrtdanawsiz62

    This (common) problem is caused by writing to a socket (or a file) bytes derived from a structure
    which has alignment padding, which is uninitialised. That is harmless, but the problem is I don't
    know of a general way to suppress these errors which doesn't also potentially hide real bugs when
    you mistakenly write uninitialised data to a file/ socket. The best I can suggest is to do a
    case-by-case suppression yourself (--gen-suppressions=yes is your friend).

### Sample Handlers ###

From inside of lib run 'make test' to build all the handlers.

#### Testing null_handler
I use three terminal sessions:

    cd deployment && ./config.sh && ./start.sh # Mongrel2 is up
    cd handler && make test && ./null_handler
    curl localhost:6767/dev/zero -v

**The curl session should return a 204 No Content success code with no content whatsoever**

#### Testing body_toupper_handler
I use three terminal sessions:

    cd deployment && ./config.sh && ./start.sh # Mongrel2 is up
    cd handler && make test && ./body_toupper_handler
    curl localhost:6767/body_to_upper_handler -d "hello handler" -v

**The curl session should spit back the data but capitalized.**

#### Testing fifo_reader_handler
Now we'll use four sessions

    make handlers && ./fifo_reader_handler
    curl http://localhost:6767/fifo_reader_handler
    cd handler && cat Makefile > handler_pipe

You will see the Makefile in the curl session.

####Test the WebSocket support

    cd deployment && ./config.sh && ./start.sh
    make handlers && ./ws_handshake_handler tcp://127.0.0.1:7999 tcp://127.0.0.1:7998
    # Use Chrome 14+ or Firefox 6+, go to http://localhost:6767/ws.html

### Contact ###
Feel free to send me through github. Patches are welcome (and how!)! I'm also on mongrel2@librelist.com.

Author: Xavier Lange
License: My code is MIT. My websocket code uses the sha1 function from PolarSSL -- that one is GPL.
