import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "../values"
Rectangle {
    id: root
    anchors.fill: parent
    property int height_min: 48
    property int index_clicked: 0
    property int  anthology_count: 0
    color: "transparent"
    function close(){
        visible = false
    }
    function open(){
        visible = true
    }
    function flip(){
        visible = !visible
    }
    function seek(position){
        var value = position / progress_bar.width
        backend.seek(value)
    }
    FileDialog {
        id:file
        title: "选择文件"
        currentFolder:StandardPaths.standardLocations(StandardPaths.DownloadLocation)
        fileMode: FileDialog.OpenFiles
        nameFilters: ["所有文件 (*.*)"]
        onAccepted: {
            if(index_clicked == 0)
                backend.open(selectedFile)
            else if(index_clicked == 2)
                backend.open_ffmpeg_opencv(selectedFile)
        }
        onRejected: {
            console.log("Canceled");
        }

    }
    Drawer_h {
        id: speed
        edge: Qt.RightEdge
        width: parent.width/3
        Rectangle {
            color: "transparent"
            anchors.fill: parent
            ListView {
                id: listview
                anchors.centerIn: parent
                width: parent.width
                height: listview.count * 40
                spacing: 16
                clip: true
                model: Speed {}
                delegate: Rectangle {
                    height: 24
                    width: parent.width
                    Text {
                        text: name + "x"
                        color: "white"
                        font.pointSize: 12
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                    }
                    MouseArea_ {
                        base_color: "transparent"
                        onClicked: {
                            backend.speed(name)
                            speed.close()
                            root.open()
                        }
                    }
                }
            }
        }
    }

    Drawer_h {
        id: anthology
        edge: Qt.RightEdge
        width: parent.width/3
        Rectangle {
            color: "transparent"
            anchors.fill: parent
            ListView {
//                id: listview
                anchors.centerIn: parent
                width: parent.width
                height: count * 40
                spacing: 16
                clip: true
                model: anthology_count
                delegate: Rectangle {
                    height: 24
                    width: parent.width
                    Text {
                        text: backend.exec("select media id " + index)
                        color: "white"
                        font.pointSize: 12
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                    }
                    MouseArea_ {
                        base_color: "transparent"
                        onClicked: {
                            backend.exec("play media id " + index)
                            anthology.close()
                            root.open()
                        }
                    }
                }
            }
        }
        onOpened: {
            anthology_count = backend.exec("count media")
        }
    }

    Drawer_h {
        id: scaler
        edge: Qt.RightEdge
        width: parent.width/3
        Rectangle {
            color: "transparent"
            anchors.fill: parent
            ListView {
//                id: listview
                anchors.centerIn: parent
                width: parent.width
                height: count * 40
                spacing: 16
                clip: true
                model: ["360p","480p","720p","1080p"]
                delegate: Rectangle {
                    height: 24
                    width: parent.width
                    Text {
                        text: modelData
                        color: "white"
                        font.pointSize: 12
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                    }
                    MouseArea_ {
                        base_color: "transparent"
                        onClicked: {
                            backend.exec("scale " + index)
                            scaler.close()
                            root.open()
                        }
                    }
                }
            }
        }
        onOpened: {
            anthology_count = backend.exec("count media")
        }
    }

    Rectangle {
        id:top
        height: height_min
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 16
            Row {
                spacing: 16
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                Repeater {
                    model: ["../image/white/file.svg","../image/white/device.svg","../image/white/more.svg"]
                    delegate: Rectangle {
                        height: 32
                        width: 32
                        radius: 16
                        property int size: 24
                        anchors.verticalCenter: parent.verticalCenter
                        color: "transparent"
                        Image {
                            id: image
                            source: modelData
                            sourceSize: Qt.size(size, size)
                            anchors { centerIn: parent }

                        }
                        MouseArea_ {
                            base_color: "transparent"
                            onClicked:{
                                index_clicked = index
                                if(index === 0)
                                {
                                    file.open()
                                }
                                else if(index === 1)
                                {
                                    var component = Qt.createComponent("ServerAddress.qml")
                                    var server = component.createObject()
                                    server.show()
                                }
                                else if(index ===2)
                                {
                                    file.open()
                                }
                            }
                        }
                    }

                }
            }

        }
        gradient:Gradient {
            GradientStop {position: 1.0; color: "#03000000"}
            GradientStop {position: 0.9; color: "#09000000"}
            GradientStop {position: 0.8; color: "#0b000000"}
            GradientStop {position: 0.7; color: "#0d000000"}
            GradientStop {position: 0.6; color: "#0f000000"}
            GradientStop {position: 0.5; color: "#15000000"}
            GradientStop {position: 0.4; color: "#22000000"}
            GradientStop {position: 0.3; color: "#30000000"}
            GradientStop {position: 0.2; color: "#50000000"}
            GradientStop {position: 0.1; color: "#92000000"}
            GradientStop {position: 0.0; color: "#ff000000"}
        }
    }
    Rectangle {
        id:bottom
        height: height_min
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
//        color: "transparent"
        ProgressBar{
            id:progress_bar
            width: parent.width*0.9
            height: 8
            value:0.5
            anchors.top: parent.top
            anchors.topMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter

            background: Rectangle {   //背景项
                width: progress_bar.width
                height: progress_bar.height-4
                anchors.verticalCenter: parent.verticalCenter
                color: "orange"
                radius: 3 //圆滑度
                MouseArea_ {
                    base_color: "#66ffffff"
                    hover_color: "#aaffffff"
                    press_color: "#ffffffff"
                    onClicked: {
                        cursor.x=mouseX
                        seek(mouseX)
                    }
                }
            }

            contentItem: Item {   //内容项
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: progress_bar.visualPosition * progress_bar.width
                    height: progress_bar.height-4
                    radius: 2
//                    color: "green"
                    MouseArea_ {
                        base_color: "#ffff0000"
                        hover_color: "#99ff0000"
                        press_color: "#bbff0000"
                        onClicked: {
                            cursor.x=mouseX
                        }
                        onReleased: {
                            seek(mouseX)
                        }
                    }

                    Rectangle {
                        id: cursor
                        height: progress_bar.height
                        width: 13
                        x: parent.width
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            font.pointSize: 7
                        }

                        MouseArea_ {
                            drag.target: parent
                            base_color: "#ffffffff"
                            onReleased: {
                                seek(cursor.x)
                            }

                        }

                        Rectangle {
                            z: -1
                            color: parent.color
                            height: parent.height
                            width: parent.width
                            x: -width
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        onXChanged: {
                            var value = x / progress_bar.width
                            progress_bar.value = value
                            if(x<=0)
                                x=0
                            if(x>=progress_bar.width)
                                x=progress_bar.width

                        }

                        Connections {
                            target: backend
                            function onSync(value){
                                console.log("sync",value)
                                cursor.x = progress_bar.width*value
//                                progress_bar.value = value
                                cursor.children[0].text = (value*100).toFixed(1)+"%"
                            }
                        }

                    }
                }
            }
        }
        Rectangle {
            anchors.top: progress_bar.bottom
            anchors.topMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            color: "transparent"
            Row {
                spacing: 16
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                Repeater {
                    model: ["../image/white/pause.svg","../image/white/next_one.svg"]
                    delegate: Rectangle {
                        height: 32
                        width: 32
                        radius: 16
                        property int size: 24
                        property bool bIsPlay: false
                        anchors.verticalCenter: parent.verticalCenter
                        color: "transparent"
                        Image {
                            source: modelData
                            sourceSize: Qt.size(size, size)
                            anchors { centerIn: parent }

                        }
                        MouseArea_ {
                            base_color: "transparent"
                            onClicked:{
                                bIsPlay = !bIsPlay
                                if(index === 0 )
                                {
                                    parent.children[0].source = bIsPlay? "../image/white/play.svg":"../image/white/pause.svg"
                                    if(bIsPlay) backend.play()
                                    else backend.pause()
                                }
                            }
                        }
                        Connections {
                            target: backend
                            function onState(value){
                                if(index === 0 ){
                                    bIsPlay = value;
                                    console.log(bIsPlay)
                                    children[0].source = bIsPlay? "../image/white/play.svg":"../image/white/pause.svg"
                                }
                            }
                        }
                    }
                }
            }
            Row {
                spacing: 16
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                Repeater {
                    model: ["字幕","选集","倍速","1080p"]
                    delegate: Text_vcenter {
                        height: 24
                        text.text: modelData
                        text.color: "white"
                        MouseArea_ {
                            base_color: "transparent"
                            onClicked:{
                                if(index === 3){
                                    root.close()
                                    scaler.open()
                                }
                                if(index === 2){
                                    root.close()
                                    speed.open()
                                }
                                if(index === 1)
                                {
                                    root.close()
                                    anthology.open()
                                }
                            }
                        }
                    }
                }
            }

        }
        gradient:Gradient {
            GradientStop {position: 0.0; color: "#03000000"}
            GradientStop {position: 0.1; color: "#09000000"}
            GradientStop {position: 0.2; color: "#0b000000"}
            GradientStop {position: 0.3; color: "#0d000000"}
            GradientStop {position: 0.4; color: "#0f000000"}
            GradientStop {position: 0.5; color: "#15000000"}
            GradientStop {position: 0.6; color: "#22000000"}
            GradientStop {position: 0.7; color: "#30000000"}
            GradientStop {position: 0.8; color: "#50000000"}
            GradientStop {position: 0.9; color: "#92000000"}
            GradientStop {position: 1.0; color: "#ff000000"}
        }
    }
    MouseArea {
        z: -1
        anchors.fill: parent
        onClicked: {
            parent.flip()
        }
    }

}
