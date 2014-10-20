import QtQuick 2.0

Rectangle {
    id: arrowRoot
    implicitWidth: 20
    implicitHeight: 20
    color: "transparent"
    property color arrowColor

    Item {
        width: upStroke.width * 2
        height: 6
        anchors.centerIn: parent
        Rectangle {
            id: upStroke
            x: 0
            y: 0
            width: 6
            height: 2
            radius: 1
            smooth: true
            transform: Rotation {
                origin.x: upStroke.width
                origin.y: upStroke.height / 2
                angle: -45
            }
            color: arrowColor
        }
        Rectangle {
            anchors.left: upStroke.right
            anchors.top:  upStroke.top
            width: upStroke.width
            height: upStroke.height
            radius: upStroke.radius
            smooth: true
            transform: Rotation {
                origin.x: 0
                origin.y: upStroke.height / 2
                angle: 45
            }
            color: arrowColor
        }
    }
}
