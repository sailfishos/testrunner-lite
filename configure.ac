AC_INIT([testrunnerlite], [1.0], [meego-qa@lists.meego.com])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AM_PROG_LIBTOOL
PKG_CHECK_MODULES([JSON], [json >= 0.9])
PKG_CHECK_MODULES([CQPID], [libcqpid])
AC_OUTPUT(\
	Makefile \
	src/Makefile \
	doc/Makefile \
	tests/Makefile \
	tests/utils/Makefile \
	tests/unit/Makefile \
	tests/regression/Makefile \
	testdata/Makefile )
