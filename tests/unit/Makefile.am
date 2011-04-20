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
	    $(XML2_CFLAGS) \
            -I$(top_builddir)/src \
            -fprofile-arcs \
            -ftest-coverage \
            $(CHECK_CFLAGS)

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
                            $(CHECK_LIBS) \
                            -lgcov \
			    $(XML2_LIBS) \
                            -lcurl \
			    -ldl \
			    -luuid	

if ENABLE_EVENTS
testrunnerliteunittests_LDADD += $(top_builddir)/src/event.o \
                                 $(JSON_LIBS) \
                                 $(CQPID_LIBS)
endif

if ENABLE_LIBSSH2
testrunnerliteunittests_LDADD += $(top_builddir)/src/remote_executor_libssh2.o \
                                 $(LIBSSH2_LIBS)
AM_CFLAGS        		      += -DENABLE_LIBSSH2
endif
