Name: testrunner-lite
Version: 1.3.3
Release:1%{?dist}
Summary: Generic test executor tool
Group: Test-tools
License: LGPL 2.1
URL: http://meego.com
Source0: %{name}_%{version}+0m6.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: autoconf, doxygen, libxml2-devel, check-devel, libcurl-devel
# libxml2 and libcurl are implicit dependencies  
Requires: test-definition, openssh

%package tests
Summary: Unit tests for testrunner-lite
Requires: testrunner-lite, ci-testing

%package regression-tests
Summary: Regression tests for testrunner-lite
Requires: testrunner-lite, ci-testing

%package docs
Summary: Testrunner-lite doxygen documentation in html format

%description
Generic test executor tool

%description tests
Unit tests for testrunner-lite

%description regression-tests
Regression tests for testrunner-lite

%description docs
Testrunner-lite doxygen documentation in html format

%prep
%setup -q -n %{name}-%{version}+0m6

%build
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

%changelog
* Wed Jun 09 2010 Sami Lahtinen <ext-sami.t.lahtinen@nokia.com> 1.3.3
- Implemented:SWP#MTT-284 - Schema and regression test cases for test results xml file
* Thu Fri 04 2010 Sampo Saaristo <ext-sampo.2.saaristo@nokia.com> 1.3.2
- Fixed process control in host based testing
* Thu Jun 03 2010 Sami Lahtinen <ext-sami.t.lahtinen@nokia.com> 1.3.1
- Initial RPM packaking
