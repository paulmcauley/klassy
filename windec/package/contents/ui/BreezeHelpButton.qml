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
            margins: 2
        }
        radius: width/2.0
        color: button.pressed ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.2) :
                                button.hovered ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.5) :
                                                 "transparent"
        Behavior on color {
            enabled: root.animateButtons
            ColorAnimation { duration: root.animationDuration }
        }

        Rectangle {
            id: stroke1
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 4
            smooth: true
            height: 2
            radius: 1
            width: parent.width - 12
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: stroke0
            anchors.top: stroke1.top
            anchors.left: stroke1.left
            anchors.topMargin: 1
            anchors.leftMargin: -1
            smooth: true
            height: 0.3*stroke1.width
            radius: 1
            width: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }

        Rectangle {
            id: stroke2
            anchors.top: stroke1.top
            anchors.topMargin: 1
            anchors.right: stroke1.right
            anchors.rightMargin: -1
            smooth: true
            height: 0.8*stroke1.width
            radius: 1
            width: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: stroke3
            anchors.right: stroke2.right
            anchors.rightMargin: 1
            anchors.bottom: stroke2.bottom
            anchors.bottomMargin: -1
            smooth: true
            height: 2
            radius: 1
            width: 0.45*stroke1.width
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: stroke4
            anchors.top: stroke3.top
            anchors.topMargin: 1
            anchors.right: stroke3.left
            anchors.rightMargin: -1
            smooth: true
            height: 0.5* stroke2.height
            radius: 1
            width: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: stroke5
            anchors.top: stroke4.bottom
            anchors.left: stroke4.left
            anchors.topMargin: 1
            smooth: true
            height: 2
            width: 2
            radius: 0.5
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
    }
    Component.onCompleted: {
        if (buttonType == DecorationOptions.DecorationButtonQuickHelp) {
            visible = decoration.providesContextHelp;
        }
    }

}
