// produces wrapper to call object implementation method
var wrap = function(name)
{
    return function() {
        var obj = this.impl__;
        return obj[name].apply(obj, [].slice.call(arguments));
    }
};

// used as a tag for "copy ctor"
var copy_tag = function(obj) { return obj.cutesClass__; };

// return function constructing js object wrapping native object,
// returned from the implementation function returning this native object.
// Wrapper object is module.res_type
var wrap_construct = function(module, name, res_type)
{
    return function() {
        var obj = this.impl__;
        var ctor = module[res_type];
        return new ctor(copy_tag, obj[name].apply(obj, [].slice.call(arguments)));
    }
};

// wrapping js object method fn_name becomes method wrapping native
// object returned by native method fn_name into module.res_type
// object
var convert_result = function(module, res_type, fn_name)
{
    return function(wrapper) {
        wrapper[fn_name] = wrap_construct(module, fn_name, res_type);
    };
};

// return function constructing array of module.res_type js objects
// each wrapping native object from the array returned by
// implementation function returning this native object
var wrap_array_construct = function(module, name, res_type)
{
    return function() {
        var obj = this.impl__;
        var m = obj[name];
        var ctor = module[res_type];
        var data = m.apply(obj, [].slice.call(arguments));
        var res = [];
        for (var i = 0; i < data.length; ++i)
            res.push(new ctor(copy_tag, data[i]));
        return res;
    }
};

// wrapping js object method fn_name becomes method wrapping each
// member of native object array returned by native method fn_name
// into array of module.res_type objects
var convert_array = function(module, res_type, fn_name)
{
    return function(wrapper) {
        wrapper[fn_name] = wrap_array_construct(module, fn_name, res_type);
    };
};

// ctor prototype for js wrapper for native object
var create_proto_ctor = function(lib, cls_name, members) {
    var info, name;
    var res = function() {
        var that = this;
        for (var i = 0; i < members.length; ++i) {
            info = members[i];
            if (typeof info === "string") {
                info = [info, info];
            } else if (typeof info === "function") {
                info(that);
                continue;
            }

            var fn = wrap(info[1]);
            that[info[0]] = fn;
        }
    };
    return res;
};

// ctor of js wrapper for native object
var create_ctor = function(lib, cls_name, members) {
    var res = function(tag, obj) {
        this.impl__ = ((tag !== copy_tag)
                       ? lib(cls_name, [].slice.call(arguments))
                       : obj);
    };
    var proto_ctor = create_proto_ctor(lib, cls_name, members);
    res.prototype = new proto_ctor();
    var cls = lib[cls_name];
    if (cls) {
        var name;
        for (name in cls)
            res[name] = cls[name];
    }
    return res;
};

exports = {
    ctor: create_ctor,
    convert_result: convert_result,
    convert_array: convert_array
}
