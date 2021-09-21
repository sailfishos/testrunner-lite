Name: testrunner-lite
Version: 1.8.4
Release: 1
Summary: Generic test executor tool
License: LGPLv2
URL: https://github.com/mer-tools/testrunner-lite
Source0: %{name}-%{version}.tar.gz 
   
BuildRequires: autoconf
BuildRequires: libxml2-devel
BuildRequires: check-devel
BuildRequires: libcurl-devel
BuildRequires: libtool
BuildRequires: libuuid-devel
# libxml2 and libcurl are implicit dependencies  
Requires: test-definition
Requires: openssh
Requires: %{name}-hwinfo-sailfish = %{version}-%{release}

%package tests
Summary: Unit tests for %{name}
Requires: %{name} = %{version}-%{release}

%package regression-tests
Summary: Regression tests for %{name}
Requires: %{name} = %{version}-%{release}
Requires: libxml2
Requires: diffutils

%package doc
Summary: Testrunner-lite man documentation
Requires:  %{name} = %{version}-%{release}
Obsoletes: %{name}-docs

%package hwinfo-sailfish
Summary: Provides commands for hardware information obtaining
Requires: coreutils
Requires: which
Recommends: ssu-sysinfo
Provides: %{name}-hwinfo
Obsoletes: %{name}-hwinfo-maemo
Obsoletes: %{name}-hwinfo-meego
Obsoletes: %{name}-hwinfo-nemo

%description
Generic test executor tool.

%description tests
Unit tests for %{name}.

%description regression-tests
Regression tests for %{name}.

%description doc
Testrunner-lite man documentation.

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
make install DESTDIR=%{buildroot}

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
%{_mandir}/man1/%{name}.1.gz

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
