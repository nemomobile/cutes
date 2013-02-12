
Name:    cutes
Summary: QtScript environment and "interpreter"
Version: 0.1.3
Release: 1

License: LGPLv2
Group:	 System/Shells
URL:     http://github.com/deztructor/cutes
Source0: %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(QtGui)
BuildRequires: pkgconfig(QtDeclarative)
BuildRequires: pkgconfig(QtScript)
BuildRequires: cmake

%description
QtScript environment and "interpreter"

%package coffee-script
Summary: CoffeeScript compiler for cutes
Group: Applications/Libraries
%description coffee-script
CoffeeScript compiler for cutes

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
#install -d -D -m755 %{buildroot}%{_datadir}/cutes/
%define cuteslibdir %{_datadir}/cutes/
install -d -D -m755 %{buildroot}%{cuteslibdir}/coffee/
install -D -p -m644 coffee/coffee-*.js %{buildroot}%{cuteslibdir}/coffee/
install -d -D -m755 %{buildroot}%{_bindir}
install -D -p -m755 coffee/coffee-script-compile %{buildroot}%{_bindir}/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_mandir}/1/%{name}.1.gz

%files coffee-script
%defattr(-,root,root,-)
%{cuteslibdir}/coffee/*.js
%{_bindir}/coffee-script-compile
