var Q = require('qtcore');

var get_actor = function() {
    var actor = cutes.actor();
    actor.source = "test_bridge.js";
    return actor;
};

var actors = [];
exports.test = function(on_done) {
    var i;
    for (i = 0; i < 10; ++i) {
        fprint(2, i, "New actor");
        var actor = get_actor();
        actors.push(actor);
        var fi = new Q.FileInfo('test_bridge.js');
        actor.request("test_actor", fi.fileName(), { on_reply: function(v) {
            fprint(2, v);
        }, on_error: function(e) {
            fprint(2, "Error", e);
        }});
        fprint(2, "next");
        cutes.gc();
    }
    for (i = 0; i < actors.length; ++i) {
        actors[i].wait();
        cutes.gc();
    }
    on_done();
};

exports.test_actor = function(msg, ctx) {
    var fi = new Q.FileInfo('.');
    var d = new Q.Dir('.');
    var e = d.entryList("*", Q.Dir.Filter.AllEntries, Q.Dir.SortFlag.Name).join(',');
    cutes.gc();
    return fi.absoluteFilePath() + "/" + msg + " and " + e;
};
