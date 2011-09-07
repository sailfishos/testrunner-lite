if GENERATE_DOCS
man1_MANS = testrunner-lite.man

html-local: doxyhtml

install-data-local:
	mkdir -p $(DESTDIR)$(docdir)/
	cp -r html $(DESTDIR)$(docdir)/

doxyhtml: 
	doxygen 

distclean-local:
	rm -rf html Makefile.in

endif
