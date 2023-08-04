import QtQuick

Rectangle {
    id: root
    property int count: 0
    property int height_min:64
    property int height_max:128
    width: 256
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    color: "grey"
    ListView {
        id: listview
        anchors.fill: parent
        anchors.margins: 20
        spacing: 5
        model: 100
        clip: true
        contentWidth:root.width
        delegate:Rectangle {
            height: root.height_min
            width: listview.contentWidth
//            color: "blue"
            MouseArea_{}
        }
    }

}
