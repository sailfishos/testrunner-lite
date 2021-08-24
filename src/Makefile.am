bin_PROGRAMS = testrunner-lite
lib_LTLIBRARIES = testrunner-lite-hwinfo-sailfish.la
testrunner_lite_hwinfo_sailfish_la_SOURCES = hwinfo_sailfish.c
testrunner_lite_hwinfo_sailfish_la_LDFLAGS = -avoid-version -module -shared -fPIC

testrunner_lite_SOURCES = main.c \
	                  testdefinitionparser.c \
	                  testdefinitiondatatypes.c \
			  testresultlogger.c \
			  testdefinitionprocessor.c \
			  testmeasurement.c \
			  testfilters.c \
	                  executor.c \
			  remote_executor.c \
			  manual_executor.c \
			  hwinfo.c \
			  utils.c \
			  log.c

testrunner_lite_LDADD = $(XML2_LIBS) -lcurl -ldl -luuid

AM_CPPFLAGS = -DLIBDIR=\"$(libdir)\"
AM_CFLAGS = $(XML2_CFLAGS) -D_GNU_SOURCE -Wall

noinst_HEADERS = testrunnerlite.h \
	         testdefinitionparser.h \
	         testdefinitiondatatypes.h \
		 testresultlogger.h \
	         testdefinitionprocessor.h \
		 testmeasurement.h \
		 testfilters.h \
	         executor.h \
		 remote_executor.h \
		 manual_executor.h \
		 hwinfo.h \
		 utils.h \
		 log.h

if ENABLE_EVENTS
testrunner_lite_SOURCES += event.c
noinst_HEADERS          += event.h
testrunner_lite_LDADD   += $(JSON_LIBS) $(CQPID_LIBS)
AM_CFLAGS               += $(JSON_CFLAGS) $(CQPID_FLAGS) -DENABLE_EVENTS
endif

if ENABLE_LIBSSH2
testrunner_lite_SOURCES += remote_executor_libssh2.c
noinst_HEADERS          += remote_executor_libssh2.h
testrunner_lite_LDADD   += $(LIBSSH2_LIBS)
AM_CFLAGS               += $(LIBSSH2_CFLAGS) -DENABLE_LIBSSH2
endif

bin_SCRIPTS = run_tests.sh
