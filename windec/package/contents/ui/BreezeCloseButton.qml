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
        smooth: true
        color: button.pressed ? "#ed1515" :
                                button.hovered ? Qt.lighter("#ed1515", 1.5) :
                                                 options.fontColor
        Behavior on color {
            enabled: root.animateButtons
            ColorAnimation { duration: root.animationDuration }
        }
        Rectangle {
            id:downStroke
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: parent.width*0.225
                rightMargin: parent.width*0.225
                top: parent.top
                bottom: parent.bottom
                topMargin: (parent.height/2) - 1
                bottomMargin: (parent.height/2) - 1
            }
            color: button.hovered || button.pressed ? options.fontColor : options.titleBarColor
            radius: 1
            smooth: true
            transform: Rotation {
                origin.x: downStroke.width/2
                origin.y: 1
                angle: 45
            }
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: upStroke
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: parent.width*0.225
                rightMargin: parent.width*0.225
                top: parent.top
                bottom: parent.bottom
                topMargin: (parent.height/2) - 1
                bottomMargin: (parent.height/2) - 1
            }
            color: button.hovered || button.pressed ? options.fontColor : options.titleBarColor
            radius:1
            smooth: true
            transform: Rotation {
                origin.x: upStroke.width/2
                origin.y: 1
                angle: -45
            }
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
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
