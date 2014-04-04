var a = cutes.actor();
if (module.args.length !== 2) {
    fprint("stderr", "Need actor file name");
    cutes.exit(0);
}
a.source=module.args[1];
if (a === undefined)
    throw "Actor is not loaded";
var is_error = false;
a.error.connect(function(e) {
    is_error = true;
});
a.wait();
if (is_error)
    throw "Got error loading actor";
