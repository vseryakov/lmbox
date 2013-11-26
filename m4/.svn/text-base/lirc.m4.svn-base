dnl
dnl Search for lirc installation
dnl

AC_DEFUN([AM_PATH_LIRC],
  [AC_ARG_ENABLE(lirc,
     [  --disable-lirc          Turn off LIRC support.],
     [], enable_lirc=yes)

  if test x"$enable_lirc" = xyes; then
     have_lirc=yes
     AC_REQUIRE_CPP
     AC_CHECK_LIB(lirc_client,lirc_init,
           [AC_CHECK_HEADER(lirc/lirc_client.h, true, have_lirc=no)], have_lirc=no)

     if test "$have_lirc" = "yes"; then
        if test x"$LIRC_PREFIX" != "x"; then
           lirc_libprefix="$LIRC_PREFIX/lib"
           LIRC_CFLAGS="-I$LIRC_PREFIX/include"
        fi
        for llirc in $lirc_libprefix /lib /usr/lib /usr/local/lib; do
          AC_CHECK_FILE(["$llirc/liblirc_client.a"],
                        [ LIRC_LIBS="$llirc/liblirc_client.a"
                          LIRC_CFLAGS="-DHAVE_LIRC" ])
        done
     else
         AC_MSG_RESULT([*** LIRC client support not available, LIRC support will be disabled ***]);
     fi
  fi
  AC_SUBST(LIRC_LIBS)
  AC_SUBST(LIRC_CFLAGS)
])

