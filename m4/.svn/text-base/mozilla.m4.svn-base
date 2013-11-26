dnl
dnl Search for mozilla installation
dnl

AC_DEFUN([AM_PATH_MOZILLA],
  [AC_ARG_ENABLE(mozilla,
     [  --disable-mozilla          Turn off Mozilla support.],
     [], enable_mozilla=yes)

  if test x"$enable_mozilla" = xyes; then
     have_mozilla=no
     PKG_CHECK_MODULES(MOZILLA, firefox-gtkmozembed >= 1.0.7,
         [ MOZILLA_CFLAGS="$MOZILLA_CFLAGS -DHAVE_MOZILLA -DXTHREADS -DXUSE_MTSAFE_API"
		have_mozilla=yes ],
                have_mozilla=no)

     if test "$have_mozilla" = "no"; then
       PKG_CHECK_MODULES(MOZILLA, xulrunner-gtkmozembed >= 1.0.7,
           [ MOZILLA_CFLAGS="$MOZILLA_CFLAGS -DHAVE_MOZILLA -DXTHREADS -DXUSE_MTSAFE_API"
             MOZILLA_LIBS="$MOZILLA_LIBS -lxul -lmozjs"
	     have_mozilla=yes ],
             have_mozilla=no)
       # Issues with xulrunner, need to add absolute path
       XUL_LIBPATH=`echo $MOZILLA_CFLAGS|cut -d ' ' -f 1 |sed -e 's/\/gtkembedmoz//' -e 's/-I//g' -e 's/include/lib/g'`
       if test "x$XUL_LIBPATH" != "x"; then
         MOZILLA_LIBS="-Wl,-rpath,$XUL_LIBPATH $MOZILLA_LIBS"
       fi
     fi
     if test "$have_mozilla" = "yes"; then
       PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.8.7, have_mozilla=yes, have_mozilla=no)
     fi

     if test "$have_mozilla" = "yes"; then
       #
       # Find base includes directory, need this for KeyEvent|MouseEvent
       #
       MOZINC=`echo $MOZILLA_CFLAGS |cut -d ' ' -f 1 | sed 's/\/gtkembedmoz//'`
       MOZILLA_CFLAGS="$MOZINC $MOZINC/dom $MOZILLA_CFLAGS"
       MOZILLA_CFLAGS="$MOZILLA_CFLAGS $GTK_CFLAGS"
       MOZILLA_LIBS="$MOZILLA_LIBS $GTK_LIBS"
       AC_SUBST(MOZILLA_LIBS)
       AC_SUBST(MOZILLA_CFLAGS)
     fi
  fi
])

