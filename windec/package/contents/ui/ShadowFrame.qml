import QtQuick 2.2
import QtGraphicalEffects 1.0

Item {
    anchors.fill: parent
    property real shadowRadius: 20
    ShadowEdge {
        id: leftShadow
        anchors {
            top: parent.top
            left: parent.left
        }
        height: shadowRadius
        width: parent.height - 2*height
        transform: [
            Rotation {
                origin.x: 0
                origin.y: 0
                angle: 90
            },
            Translate {
                x: leftShadow.height
                y: leftShadow.height
            }
        ]
    }
    ShadowCorner {
        id: topLeftShadow
        anchors {
            top: parent.top
            left: parent.left
        }
        height: shadowRadius
    }
    ShadowEdge {
        id: topShadow
        anchors {
            top: parent.top
            left: parent.left
        }
        height: shadowRadius
        width: parent.width - 2*height
        transform: [
            Scale {
                yScale: -1
            },
            Translate {
                x: topShadow.height
                y: topShadow.height
            }
        ]
    }
    ShadowCorner {
        id: topRightShadow
        anchors {
            top: parent.top
            right: parent.right
        }
        height: shadowRadius
        transform: [
            Scale {
                origin.x: topRightShadow.width/2
                origin.y: topRightShadow.height/2
                xScale: -1
            }
        ]
    }
    ShadowEdge {
        id: rightShadow
        anchors {
            top: parent.top
            left: parent.right
        }
        height: shadowRadius
        width: parent.height - 2*height
        transform: [
            Scale {
                yScale: -1
            },
            Rotation {
                origin.x: 0
                origin.y: 0
                angle: 90
            },
            Translate {
                x: -rightShadow.height
                y: rightShadow.height
            }
        ]
    }
    ShadowCorner {
        id: bottomRightShadow
        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        height: shadowRadius
        transform: [
            Scale {
                origin.x: bottomRightShadow.width/2
                origin.y: bottomRightShadow.height/2
                yScale: -1
                xScale: -1
            }
        ]
    }
    ShadowEdge {
        id: bottomShadow
        anchors {
            top: parent.bottom
            left: parent.left
        }
        height: shadowRadius
        width: parent.width - 2*height
        transform: [
            Translate {
                x: bottomShadow.height
                y: -bottomShadow.height
            }
        ]
    }
    ShadowCorner {
        id: bottomLeftShadow
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
        height: shadowRadius
        transform: [
            Scale {
                origin.x: bottomLeftShadow.width/2
                origin.y: bottomLeftShadow.height/2
                yScale: -1
            }
        ]
    }
    Rectangle {
        anchors.fill: parent
        anchors.margins: shadowRadius
        color: shadowGradient.baseShadowColor
        ShadowGradient { id: shadowGradient }
    }
}
