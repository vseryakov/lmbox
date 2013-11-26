dnl
dnl Tcl config search
dnl

AC_DEFUN(AM_PATH_TCL, [

    AC_ARG_WITH(tcl, [  --with-tcl=DIR            directory containing tcl configuration (tclConfig.sh)], with_tclconfig=${withval})
    AC_MSG_CHECKING([for Tcl configuration])

    # First check to see if --with-tclconfig was specified.
    if test x"${with_tclconfig}" != x ; then
      if test -f "${with_tclconfig}/tclConfig.sh" ; then
        tclconfig=`(cd ${with_tclconfig}; pwd)`
      else
        AC_MSG_ERROR([${with_tclconfig} directory doesn't contain tclConfig.sh])
      fi
    fi
    # check in a few common install locations
    if test x"${ac_cv_c_tclconfig}" = x ; then
      for i in ${prefix}/lib /usr/lib /usr/local/lib; do
         if test -f "$i/tclConfig.sh" ; then
           tclconfig=`(cd $i; pwd)`
           break
         fi
       done
    fi

    if test x"${tclconfig}" = x ; then
      AC_MSG_WARN(Can't find Tcl configuration definitions)
      exit 1
    else
      AC_MSG_RESULT(found ${tclconfig}/tclConfig.sh)
    fi

    AC_MSG_CHECKING([for existence of ${tclconfig}/tclConfig.sh])

    if test -f "${tclconfig}/tclConfig.sh" ; then
      AC_MSG_RESULT([loading])
      . ${tclconfig}/tclConfig.sh
    else
      AC_MSG_RESULT([file not found])
    fi

    #
    # Need eval to do the TCL_DBGX substitution in the TCL_LIB_FILE variable
    #
    eval "TCL_LIB_SPEC=\"${TCL_LIB_SPEC}\""

    if test "${TCL_THREADS}" != "1"; then
      AC_MSG_RESULT([tcl should be built with --enable-threads flag])
      exit 1
    fi

    TCL_CFLAGS="${TCL_INCLUDE_SPEC} ${CFLAGS_DEFAULT} ${CFLAGS_WARNING} ${TCL_DEFS}"
    TCL_LIBS="${TCL_LIB_SPEC} ${TCL_LIBS}"

    AC_SUBST(TCL_CFLAGS)
    AC_SUBST(TCL_LIBS)

    AC_MSG_CHECKING([how to build libraries])

    AC_ARG_ENABLE(shared,
	[  --enable-shared         build and link with shared libraries [--enable-shared]],
	[tcl_ok=$enableval], [tcl_ok=yes])

    if test "${enable_shared+set}" = set; then
      enableval="$enable_shared"
      tcl_ok=$enableval
    else
      tcl_ok=yes
    fi

    if test "$tcl_ok" = "yes" ; then
      AC_MSG_RESULT([shared])
      SHARED_BUILD=1
    else
      AC_MSG_RESULT([static])
      SHARED_BUILD=0
      AC_DEFINE(STATIC_BUILD)
    fi
])
