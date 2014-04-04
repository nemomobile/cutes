cutes;
print(1, 2);
fprint("stdout", 3, 4);
globals();
module
require
exports
process
__filename
var check_names_defined = function(obj_name, obj, names) {
    var i;
    for (i = 0; i < names.length; ++i) {
        var name = names[i];
        if (obj[name] === undefined)
            throw new Error("No " + obj_name + "." + name);
    }
};
check_names_defined("cutes", cutes, ["env", "module", "os", "path", "engine"]);
check_names_defined("module", cutes.module, ["id", "filename", "loaded", "cwd", "args", "exports"]);

exports = function(msg, ctx) {
    return [msg, "END"];
};

