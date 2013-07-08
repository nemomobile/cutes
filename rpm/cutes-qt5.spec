Name:    cutes-qt5
Summary: QtScript environment and "interpreter"
Version: 0.0.0
Release: 1

License: LGPLv2
Group:	 System/Shells
URL:     http://github.com/nemomobile/cutes
Source0: %{name}-%{version}.tar.bz2

BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Script)
BuildRequires: cmake >= 2.8
Provides: cutes = %{version}
Requires: cutes-common = %{version}

%description
QtScript environment and "interpreter"

%package -n cutes-common
Requires: qtchooser
Summary: Files common for all cutes versions  
Group: System Environment/Libraries
%description -n cutes-common
%{summary}

%define qt_importdir %{_libdir}/qt5/qml

%prep
%setup -q -n %{name}-%{version}

%build
%cmake -DUSEQT=5
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
install -d -D -p -m755 %{buildroot}%{_bindir}
ln -sf %{_bindir}/qtchooser %{buildroot}/%{_bindir}/cutes

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_qt5_bindir}/cutes
%{_libdir}/libcutescript-qt5.so
%{qt_importdir}/Mer/QtScript/libqtscript.so
%{qt_importdir}/Mer/QtScript/qmldir

%files -n cutes-common
%defattr(-,root,root,-)
%{_mandir}/man1/cutes.1.gz
%{_bindir}/cutes
