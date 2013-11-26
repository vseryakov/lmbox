dnl Configure paths for XINE
dnl Test for XINE, and define XINE_CFLAGS and XINE_LIBS

AC_DEFUN([AM_PATH_XINE],
[
  AC_ARG_ENABLE(vlc, [  --enable-xine          Turn on XINE support.], [], enable_xine=yes)

  AC_ARG_WITH(xine-prefix,
     AC_HELP_STRING([--with-xine-prefix=DIR], [prefix where XINE is installed (optional)]),
        xine_config_prefix="$withval", xine_config_prefix="")

  if test x"$enable_xine" = xyes; then
    if test x$xine_config_prefix != x ; then
      xine_config_args="$xine_config_args --prefix=$xine_config_prefix"
      if test x${XINE_CONFIG+set} != xset ; then
         XINE_CONFIG=$xine_config_prefix/bin/xine-config
      fi
    fi

    AC_PATH_PROG(XINE_CONFIG, xine-config, no)
    if test "$XINE_CONFIG" != "no" ; then
      XINE_CFLAGS="-DHAVE_XINE -DXINE_ENABLE_EXPERIMENTAL_FEATURES `$XINE_CONFIG $xine_config_args --cflags`"
      XINE_LIBS=`$XINE_CONFIG $xine_config_args --libs`
    fi
    AC_SUBST(XINE_CFLAGS)
    AC_SUBST(XINE_LIBS)
  fi
])
