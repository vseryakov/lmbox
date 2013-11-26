#!/bin/bash

rm -rf autom4te.cache

aclocal -I m4 || exit $?

automake --gnu --add-missing --copy || exit $?

autoconf || exit $?

./configure

rm -rf autom4te.cache


