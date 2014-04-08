import QtQml 2.0
import Mer.Cutes 1.1

CutesAdapter {
    id: adapter
    qml: "simple_qml_adapter.qml"
    Component.onCompleted: {
        console.log("Exit CutesAdapter")
        cutes.exit(0);
    }
}

