import QtQuick 2.2

Gradient {
    property color baseShadowColor: Qt.rgba(baseShadowColor.r,
                                            baseShadowColor.g,
                                            baseShadowColor.b,
                                            0.4)
    GradientStop {
        position: 0.0
        color: baseShadowColor
    }
    GradientStop {
        position: 0.75
        color: Qt.rgba(baseShadowColor.r, baseShadowColor.g, baseShadowColor.b, 0.08)
    }
    GradientStop {
        position: 0.9
        color: Qt.rgba(baseShadowColor.r, baseShadowColor.g, baseShadowColor.b, 0.02)
    }
    GradientStop {
        position: 1.0;
        color: Qt.rgba(baseShadowColor.r, baseShadowColor.g, baseShadowColor.b, 0.0)
    }
}
