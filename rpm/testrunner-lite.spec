Name: testrunner-lite
Version: 1.8.4
Release: 1
Summary: Generic test executor tool
License: LGPLv2
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
Requires: %{name}-hwinfo-sailfish = %{version}-%{release}
Requires: libuuid

%package tests
Summary: Unit tests for %{name}
Requires: %{name} = %{version}-%{release}

%package regression-tests
Summary: Regression tests for %{name}
Requires: %{name} = %{version}-%{release}
Requires: libxml2
Requires: diffutils

%package doc
Summary: Testrunner-lite doxygen documentation in html format
Requires:  %{name} = %{version}-%{release}
Obsoletes: %{name}-docs

%package hwinfo-maemo
Summary: Provides commands for hardware information obtaining
Requires: coreutils
Provides: %{name}-hwinfo
Conflicts: %{name}-hwinfo-meego
Conflicts: %{name}-hwinfo-sailfish

%package hwinfo-meego
Summary: Provides commands for hardware information obtaining
Requires: coreutils
Provides: %{name}-hwinfo
Conflicts: %{name}-hwinfo-maemo
Conflicts: %{name}-hwinfo-sailfish

%package hwinfo-sailfish
Summary: Provides commands for hardware information obtaining
Requires: coreutils
Requires: which
Recommends: ssu-sysinfo
Provides: %{name}-hwinfo
Conflicts: %{name}-hwinfo-maemo
Conflicts: %{name}-hwinfo-meego
Obsoletes: %{name}-hwinfo-nemo

%description
Generic test executor tool.

%description tests
Unit tests for %{name}.

%description regression-tests
Regression tests for %{name}.

%description doc
Testrunner-lite doxygen documentation in html format.

%description hwinfo-maemo
Library for obtaining hardware information in maemo environment.

%description hwinfo-meego
Library for obtaining hardware information in meego environment.

%description hwinfo-sailfish
Library for obtaining hardware information in sailfish environment.

%prep
%setup -q -n %{name}-%{version}

%build
CFLAGS="-DVERSIONSTR=%{version} -ldl"
autoreconf --install
%configure
make %{?_smp_mflags}
make html %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

mv %{buildroot}%{_docdir}/%{name}{,-%{version}}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%license COPYING
%{_bindir}/%{name}
%{_bindir}/run_tests.sh

%files tests
%defattr(-,root,root,-)
%{_libdir}/%{name}-tests
%{_datadir}/%{name}-tests

%files regression-tests
%defattr(-,root,root,-)
%{_datadir}/%{name}-regression-tests

%files doc
%defattr(-,root,root,-)
# 3 files in documention causes a duplicate warning by rpmlint
%{_docdir}/%{name}-%{version}
%{_mandir}/man1/%{name}.1.gz

%files hwinfo-maemo
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/%{name}-hwinfo-maemo*

%post hwinfo-maemo
if [ ! -e %{_libdir}/%{name}-hwinfo.so ]
then
  ln -s %{_libdir}/%{name}-hwinfo-maemo.so  %{_libdir}/%{name}-hwinfo.so
fi

%postun hwinfo-maemo
rm %{_libdir}/%{name}-hwinfo.so || :

%files hwinfo-meego
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/%{name}-hwinfo-meego*

%post hwinfo-meego
if [ ! -e %{_libdir}/%{name}-hwinfo.so ]
then
  ln -s %{_libdir}/%{name}-hwinfo-meego.so  %{_libdir}/%{name}-hwinfo.so
fi

%postun hwinfo-meego
rm %{_libdir}/%{name}-hwinfo.so || :

%files hwinfo-sailfish
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/%{name}-hwinfo-sailfish*

%post hwinfo-sailfish
if [ ! -e %{_libdir}/%{name}-hwinfo.so ]
then
  ln -s %{_libdir}/%{name}-hwinfo-sailfish.so  %{_libdir}/%{name}-hwinfo.so
fi

%postun hwinfo-sailfish
rm %{_libdir}/%{name}-hwinfo.so || :
