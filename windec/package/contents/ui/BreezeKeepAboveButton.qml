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
                                button.hovered || decoration.keepAbove ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.5) :
                                                 "transparent"
        Behavior on color {
            enabled: root.animateButtons
            ColorAnimation { duration: root.animationDuration }
        }
        Rectangle {
            id:downStroke
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: 0
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: -(0.707*downStroke.width)/2 - 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id:point
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: 2
            width: 2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            smooth: true
            transform: [
                Rotation {
                    origin.x: 1
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: -(0.707*downStroke.width)/2 - 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: upStroke
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: upStroke.width
                    origin.y: 1
                    angle: -45
                },
                Translate {
                    y: -(0.707*upStroke.width)/2 - 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id:downStroke1
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: 0
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: -(0.707*downStroke1.width)/2 + 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id:point1
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: 2
            width: 2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            smooth: true
            transform: [
                Rotation {
                    origin.x: 1
                    origin.y: 1
                    angle: 45
                },
                Translate {
                    y: -(0.707*downStroke1.width)/2 + 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
        Rectangle {
            id: upStroke1
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.horizontalCenter
            height: 2
            width: (parent.width - 5)/2
            color: button.hovered || decoration.keepAbove ? options.titleBarColor : options.fontColor
            radius:1
            smooth: true
            transform: [
                Rotation {
                    origin.x: upStroke1.width
                    origin.y: 1
                    angle: -45
                },
                Translate {
                    y: -(0.707*upStroke1.width)/2 + 2
                }
            ]
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
        }
    }
}
