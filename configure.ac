#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([margo], [0.2], [],[],[]) 
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

# we should remove this soon, only needed for automake 1.10 and older
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([margo-config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CXXCPP

AC_PROG_MKDIR_P

# coreutils checks for OSX
AC_ARG_VAR([TIMEOUT], timeout program)
AC_ARG_VAR([MKTEMP], mktemp program)

if test -z "$TIMEOUT" ; then
    AC_CHECK_PROGS(TIMEOUT, [timeout gtimeout])
    if test -z "$TIMEOUT" ; then
        AC_MSG_ERROR([Could not find timeout command (can optionally provide via the TIMEOUT variable)])
    fi
else
    AC_SUBST([TIMEOUT], ["$TIMEOUT"])
fi

if test -z "$MKTEMP" ; then
    AC_CHECK_PROGS(MKTEMP, [mktemp gmktemp])
    if test -z "$MKTEMP" ; then
        AC_MSG_ERROR([Could not find mktemp command (can optionally provide via the MKTEMP variable)])
    fi
else
    AC_SUBST([MKTEMP], ["$MKTEMP"])
fi

AC_REQUIRE_CPP

AC_CHECK_SIZEOF([long int])

dnl
dnl Verify pkg-config
dnl
PKG_PROG_PKG_CONFIG
if test "x$PKG_CONFIG" == "x"; then
   AC_MSG_ERROR([Could not find pkg-config utility!])
fi

PKG_CHECK_MODULES([MERCURY],[mercury],[],
   [AC_MSG_ERROR([Could not find working mercury installation!])])
LIBS="$MERCURY_LIBS $LIBS"
CPPFLAGS="$MERCURY_CFLAGS $CPPFLAGS"
CFLAGS="$MERCURY_CFLAGS $CFLAGS"

PKG_CHECK_MODULES([ARGOBOTS],[argobots],[],
   [AC_MSG_ERROR([Could not find working argobots installation!])])
LIBS="$ARGOBOTS_LIBS $LIBS"
CPPFLAGS="$ARGOBOTS_CFLAGS $CPPFLAGS"
CFLAGS="$ARGOBOTS_CFLAGS $CFLAGS"

PC_REQUIRES="mercury argobots"

check_abt_snoozer=auto
AC_ARG_ENABLE([abt-snoozer],
    [  --enable-abt-snoozer    enable abt-snoozer (default: dynamic check)],
    [ case "${enableval}" in
     yes) check_abt_snoozer=yes ;;
     no)  check_abt_snoozer=no ;;
     *) AC_MSG_ERROR([bad value ${enableval} for --enable-abt-snoozer]) ;;
     esac],
    [])

check_abt_snoozer_status=fail
if test "x${check_abt_snoozer}" = xauto -o "x${check_abt_snoozer}" = xyes ; then
    PKG_CHECK_MODULES([ABT_SNOOZER],[abt-snoozer],
        [AC_DEFINE([HAVE_ABT_SNOOZER], [1], [Use ABT-SNOOZER])
         PC_REQUIRES="$PC_REQUIRES abt-snoozer"
         LIBS="$ABT_SNOOZER_LIBS $LIBS"
         CPPFLAGS="$ABT_SNOOZER_CFLAGS $CPPFLAGS"
         CFLAGS="$ABT_SNOOZER_CFLAGS $CFLAGS"
         AC_SUBST([PC_REQUIRES], ["$PC_REQUIRES"])
         check_abt_snoozer_status=success],
        [printf ""])
fi

if test "x${check_abt_snoozer_status}" = xfail -a "x${check_abt_snoozer}" = xyes; then
    AC_MSG_ERROR([Could not find working abt-snoozer installation!])
fi

# turn off a warning that is tripping up our use of utlist at the moment
CFLAGS="-Wno-address $CFLAGS"

# TODO: the mercury tests should probably be in their own .m4
# make sure that mercury has boost preprocessors enabled
AC_MSG_CHECKING(if Boost preprocessing is enabled in Mercury)
AC_TRY_COMPILE([
#include <mercury_macros.h>
#ifndef HG_HAS_BOOST
#error HG_HAS_BOOST not set
#endif
], [],
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no)
AC_MSG_ERROR([Boost preprocessing not enabled in Mercury])
)

AC_CONFIG_FILES([Makefile maint/margo.pc])
AC_OUTPUT
 
