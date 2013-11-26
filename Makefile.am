version = $(shell grep LMBOX_VERSION src/lmbox.h|awk '{print $$3}'|cut -c2-6)

all:
	make -C src all

install:
	make -C src install

clean:
	-rm -rf data/News/* data/DVDrip/*
	-rm -rf lmbox *~ core* autom4te.cache config.log valgrind.*
	-rm -rf src/*.o src/*~ src/lmbox xml/*~ tcl/*~ m4/*~

distclean: clean
	-rm -rf Makefile src/Makefile 
	-rm -rf config.* configure install-sh missing aclocal.m4

valgrind:
	valgrind $(VALGRIND) --num-callers=64 --show-reachable=yes --leak-check=full --leak-resolution=high --log-file=valgrind.log lmbox

tar:    clean
	cd .. && tar -czf lmbox-${version}.tar.gz lmbox
