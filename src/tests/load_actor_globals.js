var a = cutes.actor();
a.source="globals.js";
if (a === undefined)
    throw "Actor is not loaded";
var is_error = false, is_loaded = false;
a.error.connect(function(e) {
    is_error = true;
});
a.loaded.connect(function(e) {
    is_loaded = true;
});
a.wait();
if (is_error)
    throw "Got error loading actor";
if (!is_loaded)
    throw "Have not got loaded event?";

var stage = "begin";
a.send("x", {on_reply: function(x) {
    stage = (x.length === 2 && x[0] === "x" && x[1] === "END")
        ? "done"
        : {name: "wrong", reply: x};
}, on_error: function(e) {
    stage = {name: "error", error: e};
}});
a.wait();
if (stage !== "done")
    throw stage;

