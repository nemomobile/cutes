import QtQml 2.0
import Mer.Cutes 1.1

CutesAdapter {
    id: adapter
    qml: "qml_adapter_use_cutes.qml"
    Component.onCompleted: {
        console.log("Prepare to exit CutesAdapter");
        var env = adapter.env;
        if (!env)
            throw new Error("NO ENV");
        env.setInterval(function() {
            console.log("Delayed exit");
            env.exit(0);
        }, 10);
    }
}
