testrunnerlitetestdatadir =  @datadir@/testrunner-lite-tests/testdata/
testrunnerliteregressiontestdatadir = @datadir@/testrunner-lite-regression-tests/testdata/
testrunnerlitetestdata_DATA = \
			bad_output.xml \
			sample_automatic_tests.xml \
		        testrunner-tests-get.xml \
	                testrunner-tests-pre_step-fail.xml \
			bad.xml \
	                sample_input_file.xml \
			testrunner-tests-manual-emptysteps.xml \
			testrunner-tests-semantic_invalid.xml \
			entity_substitution.xml \
			sample_schema_validation.xml \
		        testrunner-tests-manual-nosteps.xml \
		        testrunner-tests-semi_auto.xml \
			filter_tests.xml \
		        simple.xml \
			testrunner-tests-manual-set.xml \
			invalid.xml \
			testrunner-tests-bg.xml \
			testrunner-tests-measurement.xml \
			usbnetworking-tests-syntax_valid.xml \
			resumetest.xml \
			testrunner-tests-environment.xml \
			testrunner-tests-non-utf8.xml \
			testrunner-tests-utf8.xml \
			utf.xml \
			unicode.txt \
			UTF-8-demo.txt 

testrunnerliteregressiontestdata_DATA =  $(testrunnerlitetestdata_DATA) \
					 measurement.txt \
					 series.txt

testrunner-tests-utf8.xml: testrunner-tests-utf8.xml.in 
	sed s,DATADIR,$(datadir),g testrunner-tests-utf8.xml.in  > testrunner-tests-utf8.xml

clean-local:
	rm -f testrunner-tests-utf8.xml
