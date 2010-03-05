testsdefinitiondir = /usr/share/testrunner-lite-tests/
testsdefinition_DATA = tests.xml

testsscriptsdir = /usr/share/testrunner-lite-tests/
testsscripts_SCRIPTS = scripts/long_output.sh

testrunnerlitetestsdir = $(libdir)/testrunner-lite-tests

testrunnerlitetests_PROGRAMS = testrunnerlitetests

datadir = .

testrunnerlitetests_SOURCES = main.c \
                              ut_testdefinitionparser.c \
                              ut_argumentparser.c \
			      ut_testresultlogger.c \
			      ut_testexecutor.c

AM_CFLAGS = -I. \
            -I/usr/include \
            -I/usr/include/libxml2 \
            -I$(top_builddir)/src \
            -fprofile-arcs \
            -ftest-coverage

testrunnerlitetests_LDFLAGS = -lxml2

testrunnerlitetests_LDADD = $(top_builddir)/src/testdefinitionparser.o \
			    $(top_builddir)/src/testdefinitiondatatypes.o \
			    $(top_builddir)/src/testresultlogger.o \
			    $(top_builddir)/src/executor.o \
                            -lcheck \
                            -lgcov

SUBDIRS = utils
