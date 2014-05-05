import QtQuick 2.0
import org.kde.kwin.decoration 0.1

DecorationButton {
    id: button
    property real size
    width: size
    height: size
    Rectangle {
        id: hover
        anchors {
            fill: parent
            margins: 2.5
        }
        radius: width/2.0
        color: button.pressed ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.2) :
                                button.hovered ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.5) :
                                                 "transparent"
        Behavior on color {
            ColorAnimation { duration: decoration.active ? root.animationDuration : 0 }
        }
        Rectangle {
            visible: !decoration.onAllDesktops
            id:pinTop
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 3
            smooth: true
            height: parent.height/2
            width: parent.width - 12
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius: 1
        }
        Rectangle {
            visible: !decoration.onAllDesktops
            id: flange
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: pinTop.bottom
            smooth: true
            height: 2
            radius: 1
            width: parent.width - 8
            color: button.hovered ? options.titleBarColor : options.fontColor
        }
        Rectangle {
            visible: !decoration.onAllDesktops
            id: pin
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: pinTop.bottom
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 2
            smooth: true
            width: 2
            radius: 1
            color: button.hovered ? options.titleBarColor : options.fontColor
        }
        Rectangle {
            visible: decoration.onAllDesktops
            anchors.fill: parent
            anchors.margins: 3
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius: width/2
            Rectangle {
                anchors.centerIn: parent
                height: 2
                width: 2
                radius: 0
                color: button.hovered ? options.fontColor : options.titleBarColor
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
