import QtQuick 2.0
import org.kde.kwin.decoration 0.1

DecorationButton {
    id: button
    property real size
    width: size
    height: size
    Rectangle {
        anchors {
            fill: parent
            margins: 2.5
        }
        radius: width/2.0
        color: button.pressed ? "#ed1515" :
                                button.hovered ? Qt.lighter("#ed1515", 1.5) :
                                                 options.fontColor
        Behavior on color {
            ColorAnimation { duration: root.animationDuration }
        }
        Rectangle {
            id:downStroke
            anchors.centerIn: parent
            height: 2
            width: parent.width - 8
            color: button.hovered || button.pressed ? options.fontColor : options.titleBarColor
            radius:1
            smooth: true
            transform: Rotation {
                origin.x: downStroke.width/2
                origin.y: 1
                angle: 45
            }
        }
        Rectangle {
            id: upStroke
            anchors.centerIn: parent
            height: 2
            width: parent.width - 8
            color: button.hovered || button.pressed ? options.fontColor : options.titleBarColor
            radius:1
            smooth: true
            transform: Rotation {
                origin.x: upStroke.width/2
                origin.y: 1
                angle: -45
            }
        }
    }

    Connections {
        target: decoration
        onAppMenuChanged: {
            if (buttonType == DecorationOptions.DecorationButtonApplicationMenu) {
                visible = decoration.appMenu;
            }
        }
    }

}
