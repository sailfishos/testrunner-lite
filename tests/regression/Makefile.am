testdefinitiondir = @datadir@/testrunner-lite-regression-tests/
testdefinition_DATA = tests.xml testdata/*.xml

testsscriptsdir = @datadir@/testrunner-lite-regression-tests/
testsscripts_SCRIPTS = regtester.sh

tests.xml: tests.xml.in 
	sed s,DATADIR,$(datadir),g tests.xml.in | sed s,BINDIR,$(bindir),g  > tests.xml

clean-local:
	rm -f tests.xml
