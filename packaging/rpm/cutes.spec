
Name:    cutes
Summary: QtScript environment and "interpreter"
Version: 0.1.3
Release: 1

License: LGPLv2
Group:	 System Environment/Tools
URL:     http://github.com/deztructor/cutes
Source0: %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(QtGui)
BuildRequires: pkgconfig(QtDeclarative)
BuildRequires: pkgconfig(QtScript)
BuildRequires: cmake

%description
QtScript environment and "interpreter"

Group: System Environment/Tools
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}

%prep
%setup -q -n %{name}-%{version}

%build
%cmake
make %{?jobs:-j%jobs}
pushd doc
gzip -c %{name}.man > %{name}.1.gz
popd

%install
rm -rf %{buildroot}

install -D -p -m755 src/%{name} %{buildroot}%{_bindir}/%{name}
install -d -D -m755 %{buildroot}%{_mandir}/1/
install -m444 doc/%{name}.1.gz %{buildroot}%{_mandir}/1/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_mandir}/1/%{name}.1.gz
