if GENERATE_DOCS
man1_MANS = testrunner-lite.man

html-local: doxyhtml

install-data-local:
	mkdir -p $(DESTDIR)/usr/share/doc/testrunner-lite-doc
	cp -r html $(DESTDIR)/usr/share/doc/testrunner-lite-doc

doxyhtml: 
	doxygen 

distclean-local:
	rm -rf html Makefile.in

endif
