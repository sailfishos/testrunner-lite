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
PKG_CHECK_MODULES([CHECK],[check])
AM_CONDITIONAL([ENABLE_EVENTS], [test "$events_feature" = "yes"])
AM_COND_IF([ENABLE_EVENTS],
           [PKG_CHECK_MODULES([JSON], [json >= 0.9])])
AM_COND_IF([ENABLE_EVENTS],
           [PKG_CHECK_MODULES([CQPID], [libcqpid])])
AC_OUTPUT(\
	Makefile \
	src/Makefile \
	doc/Makefile \
	tests/Makefile \
	tests/utils/Makefile \
	tests/unit/Makefile \
	tests/regression/Makefile \
	testdata/Makefile )
