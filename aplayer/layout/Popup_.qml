import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

Popup {
    id: root
    width: parent.width*0.66
    height: parent.height*0.75
    x: (parent.width-width)/2
    y: (parent.height-height)/2
    margins: 0
    padding: 0
    modal: false
    focus: true
    dim:true
    Overlay.modeless: Rectangle {
        color: "transparent"
    }
    background: Rectangle {
        color: "transparent"
    }
    closePolicy:Popup.CloseOnReleaseOutside
    Component.onCompleted: close()
}
