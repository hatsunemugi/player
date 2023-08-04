import QtQuick
import QtQuick.Controls
Drawer {
    id: root
    height: parent.height
    property alias background_color: background_overlay.color
    background: Rectangle {
        id: background_overlay
        color: "#99000000"
    }
    Component.onCompleted: {
        close()
    }
    closePolicy:Popup.CloseOnReleaseOutside
    Overlay.modal: Rectangle {
        color: "transparent"
    }
}
