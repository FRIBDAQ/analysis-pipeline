#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_INIT([fribanalysis],[0.1],[scientificsoftware@nscl.msu.edu],[],[http://gitlab.msu.edu/foxr/fribanalysis])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([/base/TreeParameter.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
LT_INIT

AX_CXX_COMPILE_STDCXX_11([noext])

AC_CONFIG_FILES([
	Makefile
        base/Makefile
    ])
    

AC_OUTPUT
