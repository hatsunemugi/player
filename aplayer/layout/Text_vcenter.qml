import QtQuick

Rectangle {
    height: 24
    width: text.contentWidth
    property alias text: text
    anchors.verticalCenter: parent.verticalCenter
    color: "transparent"
    Text {
        id: text
        font.pointSize: 12
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

    }
}
