var a = cutes.actor();
a.source="globals.js";
if (a === undefined)
    throw "Actor is not loaded";
var is_error = false;
a.error.connect(function(e) {
    is_error = true;
});
a.wait();
if (is_error)
    throw "Got error loading actor";
