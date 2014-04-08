import QtQml 2.0
import Mer.Cutes 1.1

CutesActor {
    id: actor
    source: "globals.js"
    Component.onCompleted: {
        actor.send("IN", { on_reply: function() {
            console.log("Normal exit")
            cutes.exit(0);
        }, on_error: function(e) {
            console.log("Error exit", e);
            cutes.exit(1);
        }});
    }
}
