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
        color: button.pressed ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.2) :
                                button.hovered ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.5) :
                                                 "transparent"
        Behavior on color {
            ColorAnimation { duration: decoration.active ? root.animationDuration : 0 }
        }
        Rectangle {
            id:downStroke
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: 0
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: decoration.maximized ? -(0.707*downStroke.width) : -(0.707*downStroke.width)/2
                }
            ]
        }
        Rectangle {
            id:point
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: 2
            width: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            smooth: true
            transform: [
                Rotation {
                    origin.x: 1
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: decoration.maximized ? -(0.707*downStroke.width) : -(0.707*downStroke.width)/2
                }
            ]
        }
        Rectangle {
            id: upStroke
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: upStroke.width
                    origin.y: 1
                    angle: -45
                },
                Translate {
                    y: decoration.maximized ? -(0.707*upStroke.width) : -(0.707*upStroke.width)/2
                }
            ]
        }
        Rectangle {
            visible: decoration.maximized
            id:downStroke1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: downStroke1.width
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: (0.707*downStroke1.width)
                }
            ]
        }
        Rectangle {
            visible: decoration.maximized
            id:point1
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: 2
            width: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            smooth: true
            transform: [
                Rotation {
                    origin.x: 1
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: (0.707*upStroke1.width)
                }
            ]
        }
        Rectangle {
            visible: decoration.maximized
            id: upStroke1
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: 0
                    origin.y: 1
                    angle: -45
                },
                Translate {
                    y: (0.707*upStroke1.width)
                }
            ]
        }
    }
}
