
Name:    qtscriptgenerator
Summary: A tool to generate Qt bindings for Qt Script
Version: 0.2.0
Release: 3%{?dist}

License: GPLv2  
Group:	 System Environment/Libraries
URL:     http://code.google.com/p/qtscriptgenerator/	
Source0: http://qtscriptgenerator.googlecode.com/files/qtscriptgenerator-src-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Patch1: qtscriptgenerator-0.1.0-gcc44.patch
Patch2: qtscriptgenerator-src-0.1.0-no_phonon.patch

## upstreamable patches
Patch50: qtscriptgenerator-src-0.1.0-qmake_target.path.patch
# needs work
Patch51: qtscriptgenerator-kde_phonon443.patch
# fix arm ftbfs, kudos to mamba
Patch52: qtscriptgenerator-0.2.0-arm-ftbfs-float.patch
## debian patches
Patch60: memory_alignment_fix.diff

## upstream patches

# explictly BR libxslt, for xsltproc
BuildRequires: libxslt
# phonon bindings currently busted, see no_phonon patch
#BuildRequires: pkgconfig(phonon)
BuildRequires: pkgconfig(QtCore)
BuildRequires: pkgconfig(QtGui)
BuildRequires: pkgconfig(QtNetwork)
BuildRequires: pkgconfig(QtOpenGL)
BuildRequires: pkgconfig(QtSql)
BuildRequires: pkgconfig(QtSvg)
BuildRequires: pkgconfig(QtUiTools)
BuildRequires: pkgconfig(QtWebKit)
BuildRequires: pkgconfig(QtXml)
BuildRequires: pkgconfig(QtXmlPatterns)

# not strictly required, but the expectation would be for the 
# bindings to be present
Requires: qtscriptbindings = %{version}-%{release}

%description
Qt Script Generator is a tool to generate Qt bindings for Qt Script.

%package -n qtscriptbindings 
Summary: Qt bindings for Qt Script
Group: System Environment/Libraries
Provides: qtscript-qt = %{version}-%{release}
%{?_qt4:Requires: qt4%{?_isa} >= %{_qt4_version}}
%description -n qtscriptbindings
Bindings providing access to substantial portions of the Qt API
from within Qt Script.


%prep
%setup -q -n %{name}-src-%{version}

%patch1 -p0 -b .gcc44
%patch2 -p1 -b .no_phonon

%patch50 -p1 -b .qmake_target.path
%patch51 -p1 -b .kde_phonon
# I *think* we can do this unconditionally, but I'd like to
# investigate more in-depth first
%ifarch %{arm}
%patch52 -p1 -b .arm_ftbfs_float
%endif

%patch60 -p1 -b .memory_alignment


%build

# workaround buildsys bogosity, see also:
# http://code.google.com/p/qtscriptgenerator/issues/detail?id=38
export INCLUDE=%{_qt4_headerdir}

pushd generator 
%{_qt4_qmake}
make %{?_smp_mflags}
./generator
popd

pushd qtbindings
%{_qt4_qmake}
make %{?_smp_mflags}
popd

pushd tools/qsexec/src
%{_qt4_qmake}
make  %{?_smp_mflags}
popd


%install
rm -rf %{buildroot} 

mkdir -p %{buildroot}%{_qt4_plugindir}/script/
# install doesn't do symlinks
cp -a plugins/script/libqtscript* \
  %{buildroot}%{_qt4_plugindir}/script/

cp -a tools/qsexec/README.TXT README.qsexec
install -D -p -m755 tools/qsexec/qsexec %{buildroot}%{_bindir}/qsexec

install -D -p -m755 generator/generator %{buildroot}%{_qt4_bindir}/generator


%clean
rm -rf %{buildroot} 


%files
%defattr(-,root,root,-)
%{_qt4_bindir}/generator

%files -n qtscriptbindings
%defattr(-,root,root,-)
%doc README
%doc LICENSE.LGPL LGPL_EXCEPTION.txt
%doc README.qsexec 
%doc doc/
%doc examples/
%{_bindir}/qsexec
%{_qt4_plugindir}/script/libqtscript_core.so*
%{_qt4_plugindir}/script/libqtscript_gui.so*
%{_qt4_plugindir}/script/libqtscript_network.so*
%{_qt4_plugindir}/script/libqtscript_opengl.so*
#{_qt4_plugindir}/script/libqtscript_phonon.so*
%{_qt4_plugindir}/script/libqtscript_sql.so*
%{_qt4_plugindir}/script/libqtscript_svg.so*
%{_qt4_plugindir}/script/libqtscript_uitools.so*
%{_qt4_plugindir}/script/libqtscript_webkit.so*
%{_qt4_plugindir}/script/libqtscript_xml.so*
%{_qt4_plugindir}/script/libqtscript_xmlpatterns.so*


%changelog
* Thu May 03 2012 Rex Dieter <rdieter@fedoraproject.org> 0.2.0-3
- pkgconfig-style deps

* Thu May 03 2012 Rex Dieter <rdieter@fedoraproject.org> 0.2.0-2
- arm_ftbfs_float patch (from mamba)

* Tue May 01 2012 Rex Dieter <rdieter@fedoraproject.org> 0.2.0-1
- 0.2.0

* Tue Feb 28 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.1.0-18
- Rebuilt for c++ ABI breakage

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.1.0-17
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Thu Dec 22 2011 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-16
- fix qt-4.8 build, omit failing QFileOpenEvent code

* Wed Nov 16 2011 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-15
- rebuild for qt48

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.1.0-14
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Tue Dec 21 2010 Rex Dieter <rdieter@fedoraproject.org> - 0.1.0-13
- disable/omit phonon binding for now (#660852)

* Sat May 08 2010 Rex Dieter <rdieter@fedoraproject.org> - 0.1.0-12
- BR: qt4-webkit-devel

* Mon Mar 01 2010 Rex Dieter <rdieter@fedoraproject.org> - 0.1.0-11
- borrow memory_alignment_fix.diff from debian (should help arm/sparc)

* Wed Nov 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 0.1.0-10 
- rebuild (qt-4.6.0-rc1, fc13+)

* Mon Oct 19 2009 Rex Dieter <rdieter@fedoraproject.org> - 0.1.0-9
- fix build (for qt-4.6.0/phonon-isms)

* Sun Jul 26 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.1.0-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Jun 09 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-7
- upstream sun_issue27 patch

* Fri Apr 10 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-6
- qtscriptbindings: Provides: qtscript-qt ...

* Tue Mar 24 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-5
- qtscriptgenerator/qtscriptbindings pkgs 
- qtscriptbindings: include docs, examples

* Mon Mar 23 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-4
- include qsexec

* Mon Mar 23 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-3
- BR: phonon-devel

* Fri Mar 20 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-2
- qt-4.5.0-7 fixed wrt phonon, drop no_phonon patch

* Fri Mar 06 2009 Rex Dieter <rdieter@fedoraproject.org> 0.1.0-1
- first try

