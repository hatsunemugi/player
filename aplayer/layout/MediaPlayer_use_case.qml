import QtQuick
import QtMultimedia
import QtQuick.Controls
import aplayer 1.0
Rectangle {
    id: root
    anchors.fill: parent
    signal onClicked
    color: "grey"
    property bool can_play_local_media: false
    Connections {
        target: backend
        function onSelect(value){
            if(value!=="")
                mediaplayer.source=value
            if(!can_play_local_media)
                can_play_local_media = true
            backend.play()
        }
        function onState(value){
            if(value){
                mediaplayer.play()
            }
            else
                mediaplayer.pause()
        }
//        function onSeek(value){
//            console.log("seek",value)
//            mediaplayer.position = value * mediaplayer.duration
//            mediaplayer.play()
//        }
    }
//    PinchArea {
//        anchors.fill: parent
//        property double previousScale: 1
//        onPinchUpdated: {
//            if(0.75>previousScale&&pinch.scale>1||previousScale>2&&pinch.scale<1||0.75<previousScale&&previousScale<2){
//                previousScale*=(1+pinch.scale)/2
//                console.log(previousScale)
//                painter.setScale(previousScale)
//            }
//        }
//        MouseArea {
//            anchors.fill: parent
//            onClicked: {
//                var x=mouseX/width
//                var y=mouseY/height
//                console.log(x,y)
//                camera.customFocusPoint=Qt.point(x, y)
//            }
//        }
//    }
    Text {
        id: fps
        x: 50
        y: 30
        z: 1
        font.pointSize: 16
        text: "60"
//        Connections {
//            target: backend.processor
//            function onFpsUpdated(value) {
//                fps.text=value
//            }
//        }
    }
    MediaPlayer {
        id: mediaplayer
        audioOutput: audio
        videoOutput: video
        onPositionChanged: (position)=>{
            backend.sync(position/duration)
        }
    }
    VideoOutput {
        id: video
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectFit
        enabled: can_play_local_media
        visible: can_play_local_media

//        Component.onCompleted: {
//            backend.processor.setSize(width,height)
//            backend.processor.camera=camera
//            backend.processor.videoSink=videoSink
//        }
    }
    AudioOutput {
        id: audio
//        enabled: can_play_local_media
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.onClicked()
        }
    }
}


