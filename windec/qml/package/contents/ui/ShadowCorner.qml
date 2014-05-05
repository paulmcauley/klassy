import QtQuick 2.2
import QtGraphicalEffects 1.0

Rectangle {
    height: 30
    width: height
    clip: true
    color: "transparent"
    RadialGradient {
        anchors.fill: parent
        horizontalOffset: width/2
        verticalOffset: height/2
        gradient: ShadowGradient{}
    }
}
