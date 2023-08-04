import QtQuick 2.15

Rectangle {
    id: root
    height: 32
    width: 32
    radius: 16
    property int size: 24
    property alias base_color: area.base_color
    property alias source: image.source
    property alias content_opacity: image.opacity
    property bool can_click: true
    signal onClicked
    anchors.verticalCenter: parent.verticalCenter
    color: "transparent"
    Image {
        id: image
        sourceSize: Qt.size(size, size)
        anchors { centerIn: parent }

    }
    MouseArea_ {
        id: area
        enabled: can_click
        onClicked:root.onClicked()
    }
}
