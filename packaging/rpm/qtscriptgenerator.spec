
Name:    qtscriptgenerator
Summary: A tool to generate Qt bindings for Qt Script
Version: 0.2.1
Release: 1

License: GPLv2
Group:	 System Environment/Libraries
URL:     http://github.com/deztructor/qtscriptgenerator
Source0: qtscriptgenerator-%{version}.tar.bz2
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

Patch0: qtscriptgenerator-src-0.1.0-qmake_target.path.patch
Patch1: qtscriptgenerator-0.2.0-arm-ftbfs-float.patch

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

%package -n qtscriptbindings-common
Summary: Qt bindings for Qt Script - common files
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-common
Common files for QtScript Qt bindings packages.

%package -n qtscript-cli
Summary: Qt Script scripts launcher
Group: System Environment/Tools
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscript-cli
Qt Script scripts launcher used to run Qt Script javascript files


%package -n qtscriptbindings-core
Summary: Qt core bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-core
Bindings providing access to core portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-gui
Summary: Qt gui bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-gui
Bindings providing access to gui portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-network
Summary: Qt network bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-network
Bindings providing access to network portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-opengl
Summary: Qt opengl bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-opengl
Bindings providing access to opengl portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-sql
Summary: Qt sql bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-sql
Bindings providing access to sql portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-svg
Summary: Qt svg bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-svg
Bindings providing access to svg portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-uitools
Summary: Qt uitools bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-uitools
Bindings providing access to uitools portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-webkit
Summary: Qt webkit bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-webkit
Bindings providing access to webkit portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-xml
Summary: Qt xml bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-xml
Bindings providing access to xml portions of the Qt API
from within Qt Script.

%package -n qtscriptbindings-xmlpatterns
Summary: Qt xmlpatterns bindings for Qt Script
Requires: qtscriptbindings-common = %{version}-%{release}
Group: System Environment/Libraries
%{?_qt:Requires: qt%{?_isa} >= %{_qt_version}}
%description -n qtscriptbindings-xmlpatterns
Bindings providing access to xmlpatterns portions of the Qt API
from within Qt Script.
%prep
%setup -q -n %{name}-%{version}

%patch0 -p1 -b .qmake_target.path

%ifarch %{arm}
%patch1 -p1 -b .arm_ftbfs_float
%endif

%build

# workaround buildsys bogosity, see also:
# http://code.google.com/p/qtscriptgenerator/issues/detail?id=38
export INCLUDE=/usr/include

export QTDIR=%{_qt_headerdir}

pushd tools/qtscript
%qmake
make %{?jobs:-j%jobs}
gzip -c qtscript.man > qtscript.1.gz
popd

pushd generator
%qmake
make %{?jobs:-j%jobs}
./generator --include-paths=%{_qt_headerdir}
popd

pushd qtbindings
%qmake
make %{?jobs:-j%jobs}
popd

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}%{_qt_plugindir}/script/
# install doesn't do symlinks
cp -a plugins/script/libqtscript* \
  %{buildroot}%{_qt_plugindir}/script/

install -D -p -m755 tools/qtscript/qtscript %{buildroot}%{_bindir}/qtscript
install -d -D -m755 %{buildroot}%{_mandir}/1/
install -m444 tools/qtscript/qtscript.1.gz %{buildroot}%{_mandir}/1/
install -d -D -m755 %{buildroot}%{_sysconfdir}/profile.d/
install -D -p -m755 generator/generator %{buildroot}%{_qt_bindir}/qtbindings-generator

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_qt_bindir}/qtbindings-generator

%files -n qtscriptbindings-common
%defattr(-,root,root,-)
%doc README
%doc LICENSE.LGPL LGPL_EXCEPTION.txt
%doc doc/
%doc examples/

%files -n qtscript-cli
%{_bindir}/qtscript
%{_mandir}/1/qtscript.1.gz


%files -n qtscriptbindings-core
%{_qt_plugindir}/script/libqtscript_core.so*

%files -n qtscriptbindings-gui
%{_qt_plugindir}/script/libqtscript_gui.so*

%files -n qtscriptbindings-network
%{_qt_plugindir}/script/libqtscript_network.so*

%files -n qtscriptbindings-opengl
%{_qt_plugindir}/script/libqtscript_opengl.so*

%files -n qtscriptbindings-sql
%{_qt_plugindir}/script/libqtscript_sql.so*

%files -n qtscriptbindings-svg
%{_qt_plugindir}/script/libqtscript_svg.so*

%files -n qtscriptbindings-uitools
%{_qt_plugindir}/script/libqtscript_uitools.so*

%files -n qtscriptbindings-webkit
%{_qt_plugindir}/script/libqtscript_webkit.so*

%files -n qtscriptbindings-xml
%{_qt_plugindir}/script/libqtscript_xml.so*

%files -n qtscriptbindings-xmlpatterns
%{_qt_plugindir}/script/libqtscript_xmlpatterns.so*
