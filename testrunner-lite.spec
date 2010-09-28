Name: testrunner-lite
Version: 1.3.13
# build.meego.com proposed patch > Release:7.1
Release:7.1
Summary: Generic test executor tool
Group: Test-tools
License: LGPL 2.1
URL: http://meego.com
Source0: testrunner-lite.tar.gz  
BuildRoot: %{_tmppath}/testrunner-lite-root  
   
BuildRequires: autoconf, doxygen, libxml2-devel, check-devel, libcurl-devel, libtool
# libxml2 and libcurl are implicit dependencies  
Requires: test-definition, openssh, testrunner-lite-hwinfo

%package tests
Summary: Unit tests for testrunner-lite
Requires: testrunner-lite, eat

%package regression-tests
Summary: Regression tests for testrunner-lite
Requires: testrunner-lite, eat, libxml2-utils

%package docs
Summary: Testrunner-lite doxygen documentation in html format

%package hwinfo-maemo
Summary: Provides commands for hardware information obtaining
Provides: testrunner-lite-hwinfo
Conflicts: testrunner-lite-hwinfo-meego

%package hwinfo-meego
Summary: Provides commands for hardware information obtaining
Provides: testrunner-lite-hwinfo
Conflicts: testrunner-lite-hwinfo-maemo

%description
Generic test executor tool

%description tests
Unit tests for testrunner-lite

%description regression-tests
Regression tests for testrunner-lite

%description docs
Testrunner-lite doxygen documentation in html format

%description hwinfo-maemo
Library for obtaining hardware information in maemo environment

%description hwinfo-meego
Library for obtaining hardware information in meego environment

%prep
# snapshot from gitorious.org webgui - unpack dir named with qa-tools prefix
%setup -n qa-tools-testrunner-lite

%build
CFLAGS=-DVERSIONSTR=%{version}
autoreconf --install
%configure
make %{?_smp_mflags}
make html %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/bin/testrunner-lite

%files tests
%defattr(-,root,root,-)
/usr/lib/testrunner-lite-tests/*
/usr/share/testrunner-lite-tests/*

%files regression-tests
%defattr(-,root,root,-)
/usr/share/testrunner-lite-regression-tests/*

%files docs
%defattr(-,root,root,-)
%doc /usr/share/doc/testrunner-lite-doc/*
/usr/share/man/man1/testrunner-lite.1.gz
# need to remove executable flag because rpmlint complains about it
%attr(644,root,root) /usr/share/doc/testrunner-lite-doc/html/installdox

%files hwinfo-maemo
%defattr(-,root,root,-)
/usr/lib/testrunner-lite-hwinfo-maemo*

%post hwinfo-maemo
ln -s /usr/lib/testrunner-lite-hwinfo-maemo.so  /usr/lib/testrunner-lite-hwinfo.so

%postun hwinfo-maemo
rm /usr/lib/testrunner-lite-hwinfo.so

%files hwinfo-meego
%defattr(-,root,root,-)
/usr/lib/testrunner-lite-hwinfo-meego*

%post hwinfo-meego
ln -s /usr/lib/testrunner-lite-hwinfo-meego.so  /usr/lib/testrunner-lite-hwinfo.so

%postun hwinfo-meego
rm /usr/lib/testrunner-lite-hwinfo.so

