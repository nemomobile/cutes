import QtQuick 1.1
import Mer.QtScript 1.1

Rectangle
{
    id : main
    width : 300
    height: 300
    QtScriptActor {
        id : actor
        source : "actor.js"
        onError : {
            console.log("ERR:")
            for (var n in error)
                console.log(n, error[n])
        }
    }

    Component.onCompleted : {
        data.append({ name: "before", value : "first item"})
        data.append({ name: "click items while", value : "loading" })
        var add_item = function (reply) {
            for (var n in reply)
                data.append({ name: n, value : reply[n] });
        };
        actor.send({ from_qml : "qml data"}, add_item
                   , null, add_item);
        data.append({ name: "after message", value : "more items to follow" })
    }

    ListModel {
        id: data
        ListElement {
            name : "initial"
            value: 1
        }
        ListElement {
            name : "initial"
            value: 2
        }
    }

    ListView {
        anchors { fill: parent }
        model : data
        delegate : Text {
            id: txt
            text: model.name + " " + model.value
            MouseArea {
                anchors.fill: parent
                onClicked : { txt.text = "CLICKED"; }
            }
        }
    }

}
