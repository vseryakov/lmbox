dnl Configure paths for VLC
dnl Test for VLC, and define VLC_CFLAGS and VLC_LIBS

AC_DEFUN([AM_PATH_VLC],
[
  AC_ARG_ENABLE(vlc, [  --enable-lirc          Turn on VLC support.], [], enable_vlc=no)

  AC_ARG_WITH(vlc-prefix, 
     AC_HELP_STRING([--with-vlc-prefix=DIR], [prefix where VLC is installed (optional)])
        vlc_config_prefix="$withval", vlc_config_prefix="")

  if test x"$enable_vlc" = xyes; then
    if test x$vlc_config_prefix != x ; then
       vlc_config_args="$vlc_config_args --prefix=$vlc_config_prefix"
       if test x${VLC_CONFIG+set} != xset ; then
          VLC_CONFIG=$vlc_config_prefix/bin/vlc-config
       fi
    fi
    AC_PATH_PROG(VLC_CONFIG, vlc-config, no)
    if test x$VLC_CONFIG != xno ; then
      VLC_CFLAGS="-DHAVE_VLC `$VLC_CONFIG --cflags`"
      VLC_LIBS="`$VLC_CONFIG --libs vlc external`"
    else
      AC_MSG_WARN([*** Unable to find VideoLan player (http://www.viedeolan.org/)])
    fi
    AC_SUBST(VLC_CFLAGS)
    AC_SUBST(VLC_LIBS)
  fi
])
