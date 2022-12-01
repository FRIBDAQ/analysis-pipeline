#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

AC_INIT(fribanalysis, 0.1, [scientificsoftware@nscl.msu.edu],[], [http://gitlab.msu.edu/foxr/fribanalysis])

AM_INIT_AUTOMAKE

PKG_PROG_PKG_CONFIG([0.29])

AC_CONFIG_SRCDIR([/base/TreeParameter.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
LT_INIT

AM_PATH_CPPUNIT

AX_CXX_COMPILE_STDCXX_11([noext])

#  Tcl compilation

PKG_CHECK_MODULES([TCL86], [tcl8.6],[], [AC_MSG_ERROR[Missing tcl8.6 libraries]])
AC_SUBST(TCL86_CFLAGS)
AC_SUBST(TCL86_LIBS)

#  Build/install libtcl++ - I think this works in out of tree builds from the
#  repo.


tclplusdir=`realpath ${srcdir}`/libtclplus

(rm -rf libtcl-build; mkdir -p  libtcl-build ; cd libtcl-build ;  ${tclplusdir}/configure --prefix=${prefix} )
(cd libtcl-build; make all install)

TCLPLUS_CFLAGS=-I${prefix}/include
TCLPLUS_LIBS="-L${prefix}/lib -ltclPlus -lException"

AC_SUBST(TCLPLUS_CFLAGS)
AC_SUBST(TCLPLUS_LIBS)


#  Make the output files:

AC_CONFIG_FILES([Makefile
        base/Makefile
	spectcl/Makefile
    ])
    


AC_OUTPUT
