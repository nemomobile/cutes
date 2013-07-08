Name:    cutes-qt4
Summary: QtScript environment and "interpreter"
Version: 0.0.0
Release: 1

License: LGPLv2
Group:	 System/Shells
URL:     http://github.com/nemomobile/cutes
Source0: %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(QtGui)
BuildRequires: pkgconfig(QtDeclarative)
BuildRequires: pkgconfig(QtScript)
BuildRequires: cmake >= 2.8
Provides: cutes = %{version}
Requires: cutes-common = %{version}

%description
QtScript environment and "interpreter"

%if %{?_qt4_importdir:1}%{!?_qt4_importdir:0}
%define _qt_importdir %{_qt4_importdir}
%endif

%define qt4_bindir %{_libdir}/qt4/bin

%prep
%setup -q -n %{name}-%{version}

%build
%cmake -DUSEQT=4
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
# used in cutes-common, see qt5 spec
rm %{buildroot}/%{_mandir}/man1/cutes.1.gz

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{qt4_bindir}/cutes
%{_libdir}/libcutescript-qt4.so
%{_qt_importdir}/Mer/QtScript/libqtscript.so
%{_qt_importdir}/Mer/QtScript/qmldir
