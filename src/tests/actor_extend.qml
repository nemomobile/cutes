import QtQml 2.0
import Mer.Cutes 1.1

CutesActor {
    id: actor
    source: "actor_extend.js"
    Component.onCompleted: {
        var Q = cutes.require("qtcore");
        actor.send("Ping", { on_reply: function(v) {
            console.log("Normal exit: got " + v)
            cutes.exit(0);
        }, on_error: function(e) {
            console.log("Error exit", e);
            cutes.exit(1);
        }});
    }
}
