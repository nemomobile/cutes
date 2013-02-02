import QtQuick 1.1
import Mer.QtScript 1.0

Item
{
    width : 256
    height: 256
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
    Component.onCompleted : {
        qtscript.script.args
        qtscript.use("qt.core")
        var name = "./basic.qml"
        var d = new QFileInfo(name)
        console.log("is ", name, "absolute?", d.isAbsolute())
        got.text = qtscript.load("basic.js")("from qml ")
        if (got.text === expected.text)
            result.text = "passed"
    }
}
