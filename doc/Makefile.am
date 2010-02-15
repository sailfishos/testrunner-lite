html-local: doxyhtml

install-html-local:
	mkdir -p $(DESTDIR)/usr/share/doc/testrunner-lite-doc
	cp -r html $(DESTDIR)/usr/share/doc/testrunner-lite-doc

doxyhtml: 
	doxygen 

distclean-local:
	rm -rf html Makefile.in

