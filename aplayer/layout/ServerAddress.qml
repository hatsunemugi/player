import QtQuick
import QtQuick.Window
import aplayer 1.0
Window {
    id: root
    height: 24
    width: 324
    Rectangle {
        anchors.fill: parent
        color: "grey"
        Row {
            anchors.centerIn: parent
            spacing: 8
            Text {
                height: root.height
                text: "服务器地址"
                verticalAlignment: Text.AlignVCenter
            }
            Rectangle {
                height: root.height
                width: 128
                color: "#66ffffff"
                border.color: "black"
                TextEdit {
                    id: ip
                    text: "127.0.0.1"
                    anchors.fill: parent
                    anchors.margins: 8
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Text {
                height: root.height
                verticalAlignment: Text.AlignVCenter
                text: "端口"
            }
            Rectangle {
                height: root.height
                width: 44
                color: "#66ffffff"
                border.color: "black"
                TextEdit {
                    id: port
                    text: "80"
                    anchors.margins: 4
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                }
            }
            Rectangle_ {
                height: root.height
                width: 32
                base_color: "grey"
                Text {
                    text: "确认"
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                onClicked: {
                    backend.connect_to(ip.text,port.text)
                }
            }
        }
    }
}
