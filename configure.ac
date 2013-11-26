AC_INIT(lmbox, 0.9, vlad@crystalballinc.com)
AM_INIT_AUTOMAKE

# Check mtsafe dns resolver
AM_GETHOSTBYNAME_R

# Socket support
AC_CHECK_LIB(socket, socket, LIBS="-lsocket $LIBS",)
AC_CHECK_LIB(nsl, gethostbyname, LIBS="-lnsl $LIBS",)

# Check Tcl
AM_PATH_TCL

AC_CHECK_HEADER(linux/videodev.h, have_v4l=yes,)
if test x$have_v4l != xyes; then
  AC_MSG_ERROR([*** Unable to find video4linux headers, should be /usr/include/linux/videodev.h])
fi

# Check Berkeley DB
AC_CHECK_LIB(db,db_env_create,have_db=yes)
if test x$have_db = xyes; then
  DB_LIBS="-ldb"
else
  AC_MSG_ERROR([*** Unable to find Berkeley DB from http://www.sleeppycat.com])
fi

AC_SUBST(DB_LIBS)
AC_SUBST(DB_CFLAGS)

# Check X11
AC_CHECK_LIB(X11, XCreateWindow, have_x11=yes,, [-L/usr/X11R6/lib])
if test x$have_db != xyes; then
  AC_MSG_ERROR([*** Unable to find X11R6, should be in /usr/X11R6])
fi

# Check Xtest extension
AC_CHECK_LIB(Xtst, XTestFakeKeyEvent, 
                    [ X11_LIBS="$X11_LIBS -lXtst"
                      X11_CFLAGS="$X11_CFLAGS -DHAVE_XTEST" ],, [-L/usr/X11R6/lib])

AC_SUBST(X11_LIBS)
AC_SUBST(X11_CFLAGS)

# Check SDL
AM_PATH_SDL(1.2.0)

# Check for the FreeType 2 library
AC_ARG_WITH(freetype-prefix,[  --with-freetype-prefix=PFX   Prefix where FREETYPE is installed (optional)],
            freetype_prefix="$withval", freetype_prefix="")
AC_ARG_WITH(freetype-exec-prefix,[  --with-freetype-exec-prefix=PFX Exec prefix where FREETYPE is installed (optional)],
            freetype_exec_prefix="$withval", freetype_exec_prefix="")

if test x$freetype_exec_prefix != x ; then
  freetype_args="$freetype_args --exec-prefix=$freetype_exec_prefix"
  if test x${FREETYPE_CONFIG+set} != xset ; then
    FREETYPE_CONFIG=$freetype_exec_prefix/bin/freetype-config
  fi
fi
if test x$freetype_prefix != x ; then
  freetype_args="$freetype_args --prefix=$freetype_prefix"
  if test x${FREETYPE_CONFIG+set} != xset ; then
    FREETYPE_CONFIG=$freetype_prefix/bin/freetype-config
  fi
fi
AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
if test x$FREETYPE_CONFIG != xno ; then
  FREETYPE_CFLAGS="`$FREETYPE_CONFIG $freetypeconf_args --cflags`"
  FREETYPE_LIBS="`$FREETYPE_CONFIG $freetypeconf_args --libs`"
else
  AC_MSG_ERROR([*** Unable to find FreeType2 library (http://www.freetype.org/)])
fi

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

# Check SDL_mixer
AC_CHECK_LIB(SDL_mixer,Mix_OpenAudio,have_SDL_mixer=yes)
if test x$have_SDL_mixer = xyes; then
  SDL_CFLAGS="$SDL_CFLAGS -DHAVE_MIXER"
  SDL_LIBS="$SDL_LIBS -lSDL_mixer"
else
  AC_MSG_WARN([*** Unable to find SDL_mixer library (http://www.libsdl.org/projects/SDL_mixer/)])
fi

# Check image formats
AC_CHECK_LIB(png, png_create_read_struct, have_libpng=yes, , -lz)
if test x$have_libpng != xyes; then
  AC_MSG_WARN([*** Unable to find PNG library (http://www.libpng.org/pub/png/libpng.html)])
fi
AC_CHECK_LIB(z, uncompress, have_libz=yes)
if test x$have_libz != xyes; then
  AC_MSG_WARN([*** Unable to find Zlib library (http://www.gzip.org/zlib/)])
fi
if test x$have_libpng = xyes -a x$have_libz = xyes; then
  IMAGE_CFLAGS="$IMAGE_CFLAGS -DLOAD_PNG"
  IMAGE_LIBS="$IMAGE_LIBS -lpng -lz"
fi
AC_CHECK_LIB(jpeg, jpeg_CreateDecompress, have_libjpeg=yes)
if test x$have_libjpeg = xyes; then
  IMAGE_CFLAGS="$IMAGE_CFLAGS -DLOAD_JPG"
  IMAGE_LIBS="$IMAGE_LIBS -ljpeg"
else
  AC_MSG_WARN([*** Unable to find JPEG library (http://www.ijg.org/)])
fi

AC_SUBST(IMAGE_CFLAGS)
AC_SUBST(IMAGE_LIBS)

# Check ALSA
AC_CHECK_LIB(asound, snd_mixer_open, [ SDL_LIBS="$SDL_LIBS -lasound" SDL_CFLAGS="$SDL_CFLAGS -DHAVE_ALSA" ])

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

# Check xinelib
AM_PATH_XINE

# Check vlc
AM_PATH_VLC

# Check lirc
AM_PATH_LIRC

# Check mozilla
AM_PATH_MOZILLA

AC_OUTPUT(Makefile src/Makefile)

