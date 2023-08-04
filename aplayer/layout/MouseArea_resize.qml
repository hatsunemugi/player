import QtQuick 2.15
import QtQuick.Controls 6.4

MouseArea {
    anchors.fill: parent
    hoverEnabled: true
    property double x_
    property double dx
    property double y_
    property double dy
    property double px
    property double py
    property var movement: new Array(4)
    property string type
    property bool locked: false
    property bool width_locked: false
    property bool height_locked: false
    signal move

    onMove: {
        if(width_locked||height_locked){
            var RB="RB"
            if(!RB.match(type)){
                width_locked= false
                height_locked= false
            }
            else{
                var type_=get_type()
                if(RB.match(type_)){
//                    console.log(type,type_)
                    width_locked= false
                    height_locked= false
                }
            }

        }

        if(parent.width>160||movement[0]*dx>=0){
            if(!width_locked)
                parent.width+=movement[0]*dx
        }
        else
            width_locked=true

        if(parent.height>128||movement[1]*dy>=0){
            if(!height_locked)
                parent.height+=movement[1]*dy
        }
        else
            height_locked=true

        parent.x+=movement[2]*px
        parent.y+=movement[3]*py
    }
    onEntered: {
        if(cursorShape==Qt.ArrowCursor)
            type=get_type()
    }
    onReleased: cursorShape=Qt.ArrowCursor
    onPositionChanged: {
        dx=mouseX-x_
        dy=mouseY-y_
        px+=dx
        py+=dy
        x_=mouseX
        y_=mouseY
        movement=[0,0,0,0];
        if(!pressed){
            type=get_type()
        }
        else
        {
            //[width,height,x,y]
            switch(type)
            {
            case "RB":
                movement=[1,1,0,0];
                break;
            case "LT":
                movement=[-1,-1,1,1];
                dx=px;
                dy=py;
                break;
            case "RT":
                movement=[1,-1,0,1];
                dy=py;
                break;
            case "LB":
                movement=[-1,1,1,0];
                dx=px;
                break;
            case "L":
                movement=[-1,0,1,0];
                dx=px;
                break;
            case "R":
                movement=[1,0,0,0];
                break;
            case "T":
                movement=[0,-1,0,1];
                dy=py;
                break;
            case "B":
                movement=[0,1,0,0];
                break;
            default:
                break;
            }
            move();
        }
    }
    function get_type(){
        var X=parent.width
        var Y=parent.height
        if(Math.abs(mouseX-X)<20&&Math.abs(mouseY-Y)<10||Math.abs(mouseX-X)<10&&Math.abs(mouseY-Y)<20)
        {
            cursorShape=Qt.SizeFDiagCursor
            return "RB";
        }
        else if(Math.abs(mouseX-0)<10&&Math.abs(mouseY-0)<10)
        {
            cursorShape=Qt.SizeFDiagCursor
            return "LT"
        }
        else if(Math.abs(mouseX-X)<10&&Math.abs(mouseY-0)<10)
        {
            cursorShape=Qt.SizeBDiagCursor
            return "RT"
        }
        else if(Math.abs(mouseX-0)<10&&Math.abs(mouseY-Y)<10)
        {
            cursorShape=Qt.SizeBDiagCursor
            return "LB"
        }
        else if(Math.abs(mouseX-0)<10||Math.abs(mouseX-X)<10)
        {
            cursorShape=Qt.SizeHorCursor
            if(Math.abs(mouseX-0)<10)
                return "L"
            else if(Math.abs(mouseX-X)<10)
                return "R"
        }
        else if(Math.abs(mouseY-0)<10||Math.abs(mouseY-Y)<10)
        {
            cursorShape=Qt.SizeVerCursor
            if(Math.abs(mouseY-0)<10){
                return "T"
            }
            else if(Math.abs(mouseY-Y)<10){
                return "B"
            }
        }
        else
        {
            cursorShape=Qt.ArrowCursor
            return "N"
        }
    }
}
