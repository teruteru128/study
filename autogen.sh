#!/bin/sh
gettextize -f
aclocal --install
autoreconf -vfi
