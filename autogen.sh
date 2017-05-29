#!/bin/sh

echo n | libtoolize --copy --force
aclocal
autoconf
automake --add-missing 
autoheader
