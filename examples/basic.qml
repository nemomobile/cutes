import QtQuick 2.1
import Mer.Cutes 1.1

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

    CutesAdapter { qml : "basic.qml" }

    Component.onCompleted : {
        var Q = cutes.extend("qt.core")
        var name = "./basic.qml"
        var d = new Q.FileInfo(name)
        console.log("is ", name, "absolute?", d.isAbsolute())
        got.text = cutes.include("basic.js")("from qml ")
        if (got.text === expected.text)
            result.text = "passed"
    }
}
