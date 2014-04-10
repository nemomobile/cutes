import QtQml 2.0
import Mer.Cutes 1.1

CutesAdapter {
    id: adapter
    qml: "test_bridge.qml"
    Component.onCompleted: {
        var lib = cutes.require('test_bridge.js');
        lib.test(function() {
            cutes.exit(0);
        });
    }
}
