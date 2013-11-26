dnl
dnl Check to see what variant of gethostbyname_r() we have.  Defines
dnl HAVE_GETHOSTBYNAME_R_{6, 5, 3} depending on what variant is found.
dnl
dnl Based on David Arnold's example from the comp.programming.threads
dnl FAQ Q213.
dnl

AC_DEFUN([AM_GETHOSTBYNAME_R],
[saved_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -lnsl"
AC_CHECK_FUNC(gethostbyname_r, [
  AC_MSG_CHECKING([for gethostbyname_r with 6 args])
  AC_TRY_COMPILE([
    #include <netdb.h>
  ], [
    char *name;
    struct hostent *he, *res;
    char buffer[2048];
    int buflen = 2048;
    int h_errnop;

    (void) gethostbyname_r(name, he, buffer, buflen, &res, &h_errnop);
  ], [
    NET_CFLAGS="$NET_CFLAGS -DHAVE_GETHOSTBYNAME_R -DHAVE_GETHOSTBYNAME_R_6"
    AC_SUBST(NET_CFLAGS)
    AC_SUBST(NET_LIBS)
    AC_MSG_RESULT(yes)
  ], [
    AC_MSG_RESULT(no)
    AC_MSG_CHECKING([for gethostbyname_r with 5 args])
    AC_TRY_COMPILE([
      #include <netdb.h>
    ], [
      char *name;
      struct hostent *he;
      char buffer[2048];
      int buflen = 2048;
      int h_errnop;

      (void) gethostbyname_r(name, he, buffer, buflen, &h_errnop);
    ], [
      NET_CFLAGS="$NET_CFLAGS -DHAVE_GETHOSTBYNAME_R -DHAVE_GETHOSTBYNAME_R_5"
      AC_SUBST(NET_CFLAGS)
      AC_SUBST(NET_LIBS)
      AC_MSG_RESULT(yes)
    ], [
      AC_MSG_RESULT(no)
      AC_MSG_CHECKING([for gethostbyname_r with 3 args])
      AC_TRY_COMPILE([
        #include <netdb.h>
      ], [
        char *name;
        struct hostent *he;
        struct hostent_data data;

        (void) gethostbyname_r(name, he, &data);
      ], [
        NET_CFLAGS="$NET_CFLAGS -DHAVE_GETHOSTBYNAME_R -DHAVE_GETHOSTBYNAME_R_3"
        AC_SUBST(NET_CFLAGS)
        AC_SUBST(NET_LIBS)
        AC_MSG_RESULT(yes)
      ], [
        AC_MSG_RESULT(no)
      ])
    ])
  ])
])
CFLAGS="$saved_CFLAGS"])

