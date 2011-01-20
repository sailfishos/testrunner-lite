testsdefinitiondir = /usr/share/testrunner-lite-tests/
testsdefinition_DATA = tests.xml

testrunnerliteunittestsdir = $(libdir)/testrunner-lite-tests

testrunnerliteunittests_PROGRAMS = testrunnerliteunittests

datadir = .

testrunnerliteunittests_SOURCES = main.c \
                              ut_testdefinitionparser.c \
                              ut_argumentparser.c \
                              ut_testresultlogger.c \
                              ut_testexecutor.c \
                              ut_features.c \
			      ut_filters.c \
                              ut_manual_executor.c

AM_CFLAGS = -I. \
            -I/usr/include \
            -I/usr/include/libxml2 \
            -I$(top_builddir)/src \
            -fprofile-arcs \
            -ftest-coverage

testrunnerliteunittests_LDADD = $(top_builddir)/src/testdefinitionparser.o \
			    $(top_builddir)/src/testdefinitiondatatypes.o \
			    $(top_builddir)/src/testresultlogger.o \
			    $(top_builddir)/src/testdefinitionprocessor.o \
			    $(top_builddir)/src/remote_executor.o \
			    $(top_builddir)/src/manual_executor.o \
			    $(top_builddir)/src/testmeasurement.o \
			    $(top_builddir)/src/executor.o \
			    $(top_builddir)/src/hwinfo.o \
			    $(top_builddir)/src/log.o \
			    $(top_builddir)/src/utils.o \
                            -lcheck \
                            -lgcov \
                            -lxml2 \
                            -lcurl \
			    -lutil

