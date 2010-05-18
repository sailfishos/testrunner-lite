bin_PROGRAMS = testrunner-lite

testrunner_lite_SOURCES = main.c \
	                  testdefinitionparser.c \
	                  testdefinitiondatatypes.c \
			  testresultlogger.c \
			  testfilters.c \
	                  executor.c \
			  remote_executor.c \
			  manual_executor.c \
			  hwinfo.c \
			  log.c

testrunner_lite_LDFLAGS = -lxml2 -lcurl

AM_CFLAGS =  -I/usr/include/libxml2 -D_GNU_SOURCE

noinst_HEADERS = testrunnerlite.h \
	         testdefinitionparser.h \
	         testdefinitiondatatypes.h \
		 testresultlogger.h \
		 testfilters.h \
	         executor.h \
		 remote_executor.h \
		 manual_executor.h \
		 hwinfo.h \
		 log.h
