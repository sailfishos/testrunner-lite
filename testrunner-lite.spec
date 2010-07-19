Name: testrunner-lite
Version: 1.3.8
Release:1%{?dist}
Summary: Generic test executor tool
Group: Test-tools
License: LGPL 2.1
URL: http://meego.com
Source0: %{name}_%{version}+0m6.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: autoconf, doxygen, libxml2-devel, check-devel, libcurl-devel, libtool
# libxml2 and libcurl are implicit dependencies  
Requires: test-definition, openssh, testrunner-lite-hwinfo

%package tests
Summary: Unit tests for testrunner-lite
Requires: testrunner-lite, ci-testing

%package regression-tests
Summary: Regression tests for testrunner-lite
Requires: ci-testing, testrunner-lite, libxml2-utils

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
%setup -q -n %{name}-%{version}+0m6

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

%changelog
* Mon Jul 19 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.8
- Implemented:SWP#331 - Ensuring syslog synchronization
- Implemented:SWP#274 - Display version information of testrunner-lite
- Modified return values so that they reflect different error situations
-  Dependecy to sysinfo-tool moved to hwinfo-maemo package
* Mon Jul 05 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.7
- Fixed hwinfo bug
- Fixes: NB#176572 - The order of execution of post_steps and get tag
- Fixes: NB#177206 - Domain and feature attributes to result XML
- Fixes: NB#177969 - Several stuck ssh processes on worker
- Fixes: NB#176103 - Executing local test steps (removed -l shell command)
* Wed Jun 23 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.6
- Implemented:SWP#MTT-282 - hwinfo interface to provide hw information
* Tue Jun 22 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.5
- Fixes: NB#173043 - failure_info attribute is not added to case element
- Fixes: NB#172872 - Pre-steps can't time out
* Wed Jun 16 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.4
- Fixed bug in long commands handling
* Wed Jun 09 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.3-1
- Support for -H, --no-hwinfo commandline option
* Wed Jun 09 2010 Sami Lahtinen <ext-sami.t.lahtinen@nokia.com> 1.3.3
- Implemented:SWP#MTT-284 - Schema and regression test cases for test results xml file
* Thu Jun 04 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.2
- Fixed process control in host based testing
* Thu Jun 03 2010 Sami Lahtinen <ext-sami.t.lahtinen@nokia.com> 1.3.1
- Initial RPM packaking
