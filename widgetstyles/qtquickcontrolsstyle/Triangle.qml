import QtQuick 2.0

Item {
    width: 100
    height: 100
    clip: true
    property real sideLength: 70.7
    property alias color: __triangleSubRect.color
    function updateClipRect() {
        width = sideLength/Math.cos(Math.PI/4);
        height = sideLength/Math.cos(Math.PI/4);
    }

    Rectangle {
        id:__triangleSubRect
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.sideLength
        height: parent.sideLength
        color: "red"
        transform: Rotation {
            origin.x: 0
            origin.y: 0
            angle: -45
        }
    }
    transform: [Rotation {
            origin.x: 0
            origin.y: 0
            angle: -45
        },
        Translate {
            y: width
        }
    ]

    onSideLengthChanged: updateClipRect()
}
