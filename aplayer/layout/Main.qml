import QtQuick
import QtQuick.Window
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
//    MediaList{}
//    AlphaPicker {}
    Painter {
        ToolBar {
            id: toolbar
        }
        onClicked: {
            toolbar.visible = true
        }
    }


}
