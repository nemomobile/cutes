#!/usr/bin/python
import sys
import re

current_class = None
added_classes = {}
cpp_impl_methods = []

collected_data = []
pending_data = []
namespace_name = None
pending_after_namespace = []

enum_format = '''
    Q_ENUMS({name})
    enum {name} {{
        {items}
    }};
'''

std_ctor_format = '''
    Q_OBJECT
public:
    typedef JsObject<{name}, {impl}> base_type;
    static QString className() {{ return "{name}"; }}

    virtual ~{name} () {{}}

    {name}(QJSEngine &engine, QVariantList const &params);
    {name}({name} &&from) : JsObject(std::move(from)) {{}}
'''

std_ctor_impl_format = '''
{name}::{name}(QJSEngine &engine, QVariantList const &params)
    : JsObject(engine, params)
{{
}}'''

proxy_fn_format = '''
    Q_INVOKABLE {retval} {name}({src_decl_params}){const};'''

proxy_fn_impl_format = '''
{retval} {cls}::{name}({src_params}){const}
{{
    {ret_stmt}impl_->{name}({dst_params});
}}'''

copy_ctor_format = '''
    {name}(QJSEngine &e, {impl} &&src);'''

copy_ctor_impl_format = '''
{cls}::{name}(QJSEngine &e, {impl} &&src)
    : JsObject(e, std::make_shared<{impl}>(std::move(src)))
{{
}}'''

proxy_obj_p0_format = '''
    Q_INVOKABLE {retval} {name}(QVariant const &);'''

proxy_obj_p0_impl_format = '''
{retval} {cls}::{name}(QVariant const &p0)
{{
    auto p = castCutesObj<{type}>(p0);
    if (!p)
        return {errval};
    {ret_stmt}impl_->{name}(*(p->impl_));
}}'''

get_cutes_obj_format = '''
    Q_INVOKABLE QJSValue {name}({src_decl_params}){const};'''

get_cutes_obj_impl_format = '''
QJSValue {cls}::{name}({src_params}){const}
{{
    return convert<QJSValue>({type}(engine_, impl_->{name}({dst_params})));
}}
'''

traits_format = '''
template <>
struct JsTraits<{impl}>
{{
    typedef {name} js_type;
}};
'''

meta_format = '''Q_DECLARE_METATYPE({name}*);
'''

class_fwd_format = '''class {name};
'''

cpp_intro_format = '''// -----------------------------------------------------------------------------
// THIS IS AUTOMATICALLY GENERATED FILE! DO NOT EDIT IT DIRECTLY!
// Original source file is {name}.cpp.in
// -----------------------------------------------------------------------------
'''

def format_params(params):
    if params is None:
        params = []
    i = 0
    src = []
    src_decl = []
    dst = []
    for v in params:
        name = 'p{}'.format(i)
        d = name
        if type(v) == str:
            p = ' '.join([v, name])
            src.append(p)
            src_decl.append(p)
        else:
            s = ' '.join([v["sig"], name])
            s_decl = s
            # TODO should be done in js wrapper?
            # if "default" in v:
            #     s_decl = ' '.join([s_decl, "=", str(v["default"])])
            src.append(s)
            src_decl.append(s_decl)
            if "convert" in v:
                d = v["convert"].format(d)
        dst.append(d)
        i += 1
    return (', '.join(src), ', '.join(dst), ', '.join(src_decl))

def collect_pending_data():
    global pending_data, collected_data
    if len(pending_data):
        collected_data.extend(pending_data)
        pending_data = []

def format_all(src, **kwargs):
    global current_class
    return [x.format(cls = current_class, **kwargs) for x in src]

def format_decl_add_impl(decl_, impl_, **kwargs):
    decl_, impl_ = format_all((decl_, impl_), **kwargs)
    global cpp_impl_methods
    cpp_impl_methods.append(impl_)
    return decl_

def cpp_intro(name):
    return cpp_intro_format.format(name = name)

def cpp_impl():
    global cpp_impl_methods
    collect_pending_data()
    res = '\n'.join(cpp_impl_methods)
    cpp_impl_methods = []
    return res

def namespace(ns):
    global namespace_name
    namespace_name = ns

def before_decl():
    collect_pending_data()

def before_def_out_namespace():
    global pending_data, pending_after_namespace
    pending_data.extend(pending_after_namespace)
    pending_after_namespace = []

def aclass(name, impl):
    global current_class, class_members, aclass_format, namespace_name, collected_data
    collected_data.append(class_fwd_format.format(name = name))
    pending_after_namespace.append(
        meta_format.format(name = '::'.join([namespace_name, name])))

    current_class = name
    added_classes[name] = []
    return traits_format.format(name = name, impl = impl)

def add_class_member(name, info):
    info["name"] = name
    added_classes[current_class].append(info)

def custom_fn(name, js = None):
    if js is None:
        js = {}
    add_class_member(name, js)

def get_cutes_obj(atype, name, params = None, is_const = False, js = None):
    if js is None:
        js = {}
    js["convert"] = "convert_result({!r}, {!r})".format(atype, name)
    add_class_member(name, js)
    src, dst, src_decl = format_params(params)
    c = " const" if is_const else ""
    return format_decl_add_impl(get_cutes_obj_format, get_cutes_obj_impl_format
                                , name = name, type = atype
                                , src_params = src, dst_params = dst
                                , src_decl_params = src_decl, const = c)

def proxy_obj_p0(retval, name, atype, errval = None, js = None):
    if js is None:
        js = {}
    add_class_member(name, js)
    if retval == "void":
        ret_stmt = ''
        errval = ''
    elif errval is None:
        raise Exception("Need errval for " + retval)
    else:
        ret_stmt = 'return '
    return format_decl_add_impl(proxy_obj_p0_format, proxy_obj_p0_impl_format
                                , retval = retval, ret_stmt = ret_stmt
                                , name = name, type = atype, errval = errval)

def enum(name, impl, names):
    names = names.split(' ')
    items = ["{name} = {impl}::{name}".format(name = x, impl = impl)
             for x in names]
    items = ','.join(items)
    global enum_format
    return enum_format.format(name = name, items = items)

def copy_ctor():
    def format_res(**kwargs):
        return format_decl_add_impl(copy_ctor_format, copy_ctor_impl_format, **kwargs)

    return format_res

def header(name, impl, more_ctors = None):
    cpp_impl_methods.append(std_ctor_impl_format.format(name = name, impl = impl))
    res = std_ctor_format.format(name = name, impl = impl)
    if more_ctors:
        aux = [eval(x) for x in more_ctors]
        aux = [x(name = name, impl = impl) for x in aux]
        res += '\n'.join(aux)
    return res

def proxy(retval, name, params = None, is_const = False, js = None):
    if js is None:
        js = {}
    add_class_member(name, js)
    src, dst, src_decl = format_params(params)
    c = " const" if is_const else ""
    ret_stmt = "" if retval == "void" else "return "
    return format_decl_add_impl(proxy_fn_format, proxy_fn_impl_format
                                , retval = retval, name = name, ret_stmt = ret_stmt
                                , src_params = src, dst_params = dst
                                , src_decl_params = src_decl, const = c)

register_class_format = '''    addClass<{name}>("{name}");
'''
def register_classes():
    return ''.join([register_class_format.format(name = c) for c in added_classes])

cmd_re = re.compile(r'^ *(//)? *@(.+)$')

with open(sys.argv[1], 'r') as infile:
    with open(sys.argv[2], 'w') as outfile:
        for l in infile:
            m = cmd_re.match(l)
            if m:
                cmd = m.group(2)
                out = eval(cmd)
            else:
                out = l
            if not out is None:
                pending_data.append(out)

collect_pending_data()
with open(sys.argv[2], 'w') as outfile:
    [outfile.write(x) for x in collected_data]
current_class = None

js_intro = '''// -----------------------------------------------------------------------------
// THIS IS AUTOMATICALLY GENERATED FILE! DO NOT EDIT IT DIRECTLY!
// Original source file is {name}.cpp.in
// -----------------------------------------------------------------------------

var W = require("wrap-qt");
var Q = cutes.extend("qt.core");

var ctor = function(name, members) {
    return W.ctor(Q, name, members);
};
var convert_result = function(res, name) {
    return W.convert_result(exports, res, name)
};

var convert_array = function(res, name) {
    return W.convert_array(exports, res, name)
};

var rename = function(from, to) {
    return [to, from];
};
'''

js_format = '''
exports.{name} = ctor("{name}",
    [{members}]);
'''

def process_js(name, members):
    def process_member(m):
        if "rename" in m:
            return "rename({!r}, {!r})".format(m["name"], m["rename"])
        elif "convert" in m:
            return m["convert"]
        else:
            return repr(m["name"])

    members = [process_member(m) for m in members]
    members = ', '.join([x for x in members])
    return js_format.format(name = name, members = members)

result_js = [process_js(c, added_classes[c]) for c in added_classes]
with open(sys.argv[3], 'w') as js_file:
    js_file.write('{}{}'.format(js_intro, '\n'.join(result_js)))
