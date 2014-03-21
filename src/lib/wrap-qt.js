var IS_DEBUG = false;

var dump, trace;
if (IS_DEBUG) {
    dump = function(n, o) {
        print(n, typeof o);
        for (var m in o) {
            print("!", m, typeof o[m]);
        }
    };
    trace = print;
} else {
    dump = function() {}
    trace = function() {}
}

var wrap = function(obj, name)
{
    trace("WRAP", obj.cutesClass__, name);
    return function() {
        var m = obj[name];
        var o = obj;
        trace("WRAPPED", typeof m, obj.cutesClass__, name);
        return m.apply(obj, [].slice.call(arguments));
    }
};

var copy_tag = function(obj) { return obj.cutesClass__; };

var wrap_construct = function(module, obj, name, res_type)
{
    var m = obj[name];
    return function() {
        var ctor = module[res_type];
        return new ctor(copy_tag, m.apply(obj, [].slice.call(arguments)));
    }
};

var wrap_array_construct = function(module, obj, name, res_type)
{
    var m = obj[name];
    return function() {
        var ctor = module[res_type];
        var data = m.apply(obj, [].slice.call(arguments));
        var res = [];
        for (var i = 0; i < data.length; ++i)
            res.push(new ctor(copy_tag, data[i]));
        return res;
    }
};

var create_ctor = function(lib, cls_name, members) {
    var name;
    dump(cls_name, lib);
    var res = function(tag, obj) {
        var that = this;
        var impl;
        if (tag === copy_tag) {
            impl = obj;
        } else {
            impl = lib(cls_name, [].slice.call(arguments));
        }
        that.impl__ = impl;
        dump(cls_name, that);
        for (var i = 0; i < members.length; ++i) {
            name = members[i];
            if (typeof name === "string") {
                name = [name, name];
            } else if (typeof name === "function") {
                name(that, impl);
                continue;
            }

            var fn = wrap(impl, name[1]);
            that[name[0]] = fn;
        }
        dump("->" + cls_name, that);
    };
    var cls = lib[cls_name];
    if (cls) {
        for (name in cls)
            res[name] = cls[name];
    }
    return res;
};

var convert_result = function(module, res_type, fn_name)
{
    return function(wrapper, obj) {
        wrapper[fn_name] = wrap_construct(module, obj, fn_name, res_type);
    };
};

var convert_array = function(module, res_type, fn_name)
{
    return function(wrapper, obj) {
        wrapper[fn_name] = wrap_array_construct(module, obj, fn_name, res_type);
    };
};

exports = {
    ctor: create_ctor,
    convert_result: convert_result,
    convert_array: convert_array
}
