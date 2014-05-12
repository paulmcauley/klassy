import QtQuick 2.2

Gradient {
    property color shadowColor: "#232629"
    property color baseShadowColor: Qt.rgba(shadowColor.r,
                                            shadowColor.g,
                                            shadowColor.b,
                                            0.35)
    GradientStop {
        position: 0.0
        color: Qt.rgba(shadowColor.r,
                       shadowColor.g,
                       shadowColor.b,
                       0.35)
    }
    GradientStop {
        position: 0.25
        color: Qt.rgba(shadowColor.r, shadowColor.g, shadowColor.b, 0.25)
    }
    GradientStop {
        position: 0.50
        color: Qt.rgba(shadowColor.r, shadowColor.g, shadowColor.b, 0.13)
    }
    GradientStop {
        position: 0.75
        color: Qt.rgba(shadowColor.r, shadowColor.g, shadowColor.b, 0.04)
    }
    GradientStop {
        position: 1.0;
        color: Qt.rgba(shadowColor.r, shadowColor.g, shadowColor.b, 0.0)
    }
}
