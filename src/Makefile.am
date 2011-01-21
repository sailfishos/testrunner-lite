bin_PROGRAMS = testrunner-lite
lib_LTLIBRARIES = testrunner-lite-hwinfo-maemo.la \
		  testrunner-lite-hwinfo-meego.la
testrunner_lite_hwinfo_maemo_la_SOURCES = hwinfo_maemo.c
testrunner_lite_hwinfo_maemo_la_LDFLAGS = -avoid-version -module -shared -fPIC
testrunner_lite_hwinfo_maemo_la_CLFAGS  = -fPIC
testrunner_lite_hwinfo_meego_la_SOURCES = hwinfo_meego.c
testrunner_lite_hwinfo_meego_la_LDFLAGS = -avoid-version -module -shared -fPIC

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
			  log.c \
			  event.c

testrunner_lite_LDADD = -lxml2 -lcurl $(JSON_LIBS) -lcqpid

AM_CFLAGS =  -I/usr/include/libxml2 -D_GNU_SOURCE $(JSON_CFLAGS)

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
		 log.h \
		 event.h
