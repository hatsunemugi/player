import QtQuick 2.15

Rectangle {
    id: root
    property alias base_color: area.base_color
    property alias hover_color: area.hover_color
    property alias press_color: area.press_color
    property alias base_color_light: area.base_color_light
    property alias base_color_dark: area.base_color_dark
    signal clicked
    MouseArea_ {
        id: area
        onClicked: root.clicked()
    }
}
