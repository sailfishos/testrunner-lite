testsdefinitiondir = /usr/share/testrunner-lite-tests/
testsdefinition_DATA = tests.xml

testrunnerlitetestsdir = $(libdir)/testrunner-lite-tests
testrunnerlitetests_DATA = testdefinition.xsd

testrunnerlitetests_PROGRAMS = testrunnerlitetests

datadir = .
 
testrunnerlitetests_SOURCES = main.c \
                              ut_testdefinitionparser.c \
                              ut_argumentparser.c

AM_CFLAGS = -I. \
            -I/usr/include \
            -I/usr/include/libxml2 \
            -I$(top_builddir)/src \
            -fprofile-arcs \
            -ftest-coverage
    
testrunnerlitetests_LDFLAGS = -lxml2

testrunnerlitetests_LDADD = $(top_builddir)/src/testdefinitionparser.o \
                            -lcheck \
                            -lgcov