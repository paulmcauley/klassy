import QtQuick 2.0
import "ColorUtils.js" as ColorUtils

Rectangle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }
    opacity: control.enabled ? 1.0 : 0.5
    anchors.centerIn: parent
    implicitWidth: styleData.horizontal ? 300 : 20
    implicitHeight: styleData.horizontal ? 20 : 300
    color: "transparent"
    property alias barColor: __barFill.color
    property alias barBorder: __barFill.border
    property real percentWidth: 0.5
    property alias showShadow: __shadow.visible
    property alias showHandle: __handle.visible
    property alias handleColor: __handle.color
    property alias handleBorder: __handle.border

    Rectangle {
        id: __barFill
        anchors.fill: parent
        anchors.leftMargin: styleData.horizontal ? 0 : 0.5 * (1 - percentWidth) * 20
        anchors.rightMargin: styleData.horizontal ? 0 : 0.5 * (1 - percentWidth) * 20
        anchors.topMargin: styleData.horizontal ? 0.5 * (1 - percentWidth) * 20 : 0
        anchors.bottomMargin: styleData.horizontal ? 0.5 * (1 - percentWidth) * 20 : 0
        color: "red"
        radius: width/2
    }

    Rectangle {
        id: __shadow
        visible: false
        anchors.centerIn: __barFill
        width: 20
        height: 20
        color: ColorUtils.adjustAlpha(sysPalette.windowText, 0.15)
        radius: width/2
        transform: Translate {
            x: 0
            y: 0
        }
    }

    Rectangle {
        id: __handle
        visible: false
        anchors.centerIn: __barFill
        width: 20
        height: 20
        color: "red"
        radius: width/2
        transform: Translate {
            x: __shadow.visible ? (styleData.pressed ? 0 : -1) : 0
            y: __shadow.visible ? (styleData.pressed ? 0 : -1) : 0
        }
    }

}
