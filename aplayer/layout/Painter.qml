import QtQuick
import QtMultimedia
import QtQuick.Controls
import aplayer 1.0
import Enums 1.0
Rectangle {
    id: root
    anchors.fill: parent
    signal clicked
    color: "grey"
    onWidthChanged: {
        painter.setSize(width,height)
    }
    onHeightChanged: {
        painter.setSize(width,height)
    }
    Rectangle {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        color: "transparent"
        PaintedItem {
            id: painter
            anchors.fill: parent
            Component.onCompleted: {
                painter.setSize(root.width,root.height)
                backend.painter = painter
                if(painter.AspectRatio === Enums.AspectRatio.autofill)
                    console.log("autofill")
            }


            Connections {
                target: painter
                function onRetarget(width_,height_)
                {
                    console.log("retarget")
                    painter.parent.width = width_
                    painter.parent.height = height_
                }
                function onAnother()
                {
                    painter.setSize(root.width,root.height)
                }
            }

        }
    }


    Text {
        id: fps
        x: 50
        y: 30
        z: 1
        font.pointSize: 16
        text: "60"
        Connections {
            target: painter
            function onFps(value) {
                fps.text=value
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.clicked()
        }
    }
}


