Name: testrunner-lite
Version: 1.8.1
Release: 1
Summary: Generic test executor tool
Group: Development/Tools
License: LGPL 2.1
URL: https://github.com/mer-tools/testrunner-lite
Source0: %{name}-%{version}.tar.gz 
   
BuildRequires: autoconf
BuildRequires: doxygen
BuildRequires: libxml2-devel
BuildRequires: check-devel
BuildRequires: libcurl-devel
BuildRequires: libtool
BuildRequires: libuuid-devel
# libxml2 and libcurl are implicit dependencies  
Requires: test-definition
Requires: openssh
Requires: testrunner-lite-hwinfo
Requires: libuuid

%package tests
Summary: Unit tests for testrunner-lite
Requires: testrunner-lite

%package regression-tests
Summary: Regression tests for testrunner-lite
Requires: testrunner-lite
Requires: libxml2
Requires: diffutils

%package docs
Summary: Testrunner-lite doxygen documentation in html format

%package hwinfo-maemo
Summary: Provides commands for hardware information obtaining
Requires: coreutils
Provides: testrunner-lite-hwinfo
Conflicts: testrunner-lite-hwinfo-meego

%package hwinfo-meego
Summary: Provides commands for hardware information obtaining
Requires: coreutils
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
# Adjusting %%setup since git-pkg unpacks to src/
# %%setup -n %%{name}-%%{version}
%setup -n src

%build
CFLAGS="-DVERSIONSTR=%{version} -ldl"
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
%{_bindir}/%{name}
%{_bindir}/run_tests.sh

%files tests
%defattr(-,root,root,-)
%{_libdir}/testrunner-lite-tests/*
%{_datadir}/testrunner-lite-tests/*

%files regression-tests
%defattr(-,root,root,-)
%{_datadir}/testrunner-lite-regression-tests/*

%files docs
%defattr(-,root,root,-)
# 3 files in documention causes a duplicate warning by rpmlint
%doc %{_docdir}/testrunner-lite/*
%{_mandir}/man1/testrunner-lite.1.gz
# need to remove executable flag because rpmlint complains about it
%attr(644,root,root) %{_docdir}/testrunner-lite/html/installdox

%files hwinfo-maemo
%defattr(-,root,root,-)
%{_libdir}/testrunner-lite-hwinfo-maemo*

%post hwinfo-maemo
ln -s %{_libdir}/testrunner-lite-hwinfo-maemo.so  %{_libdir}/testrunner-lite-hwinfo.so

%postun hwinfo-maemo
rm %{_libdir}/testrunner-lite-hwinfo.so

%files hwinfo-meego
%defattr(-,root,root,-)
%{_libdir}/testrunner-lite-hwinfo-meego*

%post hwinfo-meego
ln -s %{_libdir}/testrunner-lite-hwinfo-meego.so  %{_libdir}/testrunner-lite-hwinfo.so

%postun hwinfo-meego
rm %{_libdir}/testrunner-lite-hwinfo.so

