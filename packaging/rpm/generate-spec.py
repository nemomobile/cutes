#!/usr/bin/python

import re

pkg_tpl = '''
%package -n qtscriptbindings-{name}
Summary: Qt {name} bindings for Qt Script
Requires: qtscriptbindings-common = %{{version}}-%{{release}}
Group: System Environment/Libraries
%{{?_qt:Requires: qt%{{?_isa}} >= %{{_qt_version}}}}
%description -n qtscriptbindings-{name}
Bindings providing access to {name} portions of the Qt API
from within Qt Script.'''

file_tpl='''
%files -n qtscriptbindings-{name}
%{{_qt_plugindir}}/script/libqtscript_{name}.so*'''

packages = ['core', 'gui', 'network', 'opengl', 'sql', 'svg', 'uitools',
            'webkit', 'xml', 'xmlpatterns']

def replaced(l):
    global packages
    m = re.match(r'.*@@([a-z]+)@@.*', l)
    return '\n'.join([templates[m.group(1)].format(name = x) for x in packages]) \
        if m else l
    

templates = { 'files' : file_tpl, 'pkgs' : pkg_tpl }

with open('qtscriptgenerator.spec.tpl') as f:
    #lines = [x.strip() for x in f.readlines()]
    lines = f.readlines()
    with open('qtscriptgenerator.spec', 'w') as f:
        [f.write(replaced(l)) for l in lines]
        f.write('\n')

