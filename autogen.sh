#!/bin/sh

libtoolize -c || glibtoolize -c
autoreconf -fv --install
