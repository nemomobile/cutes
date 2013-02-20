import QtQuick 1.1
import Mer.QtScript 1.1

Rectangle
{
    width : 256
    height: 256
    color: "white"
    Column {
        Text {
            text: "Expect:"
        }
        Text {
            id: expected
            text: "from qml + line from qtscript"
        }
        Text {
            text: "Got:"
        }
        Text {
            id : got
            text: "This is not the right text"
        }
        Text {
            id : result
            text: "failed"
        }
    }

    QtScriptAdapter { qml : "basic.qml" }

    Component.onCompleted : {
        qtscript.script.args
        qtscript.extend("qt.core")
        var name = "./basic.qml"
        var d = new QFileInfo(name)
        console.log("is ", name, "absolute?", d.isAbsolute())
        got.text = require("basic.js")("from qml ")
        if (got.text === expected.text)
            result.text = "passed"
    }
}
