AC_INIT([testrunnerlite], [1.0], [meego-qa@lists.meego.com])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AM_PROG_LIBTOOL
AC_ARG_ENABLE(
  [events],
  [AS_HELP_STRING([--enable-events],
    [enable event communication [default=no]])],
  [case "$enableval" in
     yes) events_feature=yes ;;
     no)  events_feature=no ;;
     *)   AC_MSG_ERROR([Invalid value "$enableval" for --enable-events]) ;;
   esac],
  [events_feature=no]
)
AC_ARG_ENABLE(
  [libssh2],
  [AS_HELP_STRING([--enable-libssh2],
    [enable ssh connections using libssh2 [default=no]])],
  [case "$enableval" in
     yes) libssh2_feature=yes ;;
     no)  libssh2_feature=no ;;
     *)   AC_MSG_ERROR([Invalid value "$enableval" for --enable-libssh2]) ;;
   esac],
  [libssh2_feature=no]
)
PKG_CHECK_MODULES([CHECK],[check])
PKG_CHECK_MODULES([XML2],[libxml-2.0])
AM_CONDITIONAL([ENABLE_EVENTS], [test "$events_feature" = "yes"])
AM_COND_IF([ENABLE_EVENTS],
           [PKG_CHECK_MODULES([JSON], [json >= 0.9])])
AM_COND_IF([ENABLE_EVENTS],
           [PKG_CHECK_MODULES([CQPID], [libcqpid])])
AM_CONDITIONAL([ENABLE_LIBSSH2], [test "$libssh2_feature" = "yes"])
AM_COND_IF([ENABLE_LIBSSH2],
           [PKG_CHECK_MODULES([LIBSSH2], [libssh2 >= 1.2.4])])
AC_OUTPUT(\
	Makefile \
	src/Makefile \
	doc/Makefile \
	tests/Makefile \
	tests/utils/Makefile \
	tests/unit/Makefile \
	tests/regression/Makefile \
	testdata/Makefile )
