import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

SliderStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    groove: Rectangle {
        opacity: control.enabled ? 1.0 : 0.5
        id: control
        anchors.centerIn: parent
        implicitWidth: 300
        implicitHeight: 16
        color: "transparent"

        Rectangle {
            anchors.fill: parent
            anchors.margins: 5
            color: ColorUtils.adjustAlpha(sysPalette.windowText, 0.3)
            radius: 3
        }

        Rectangle {
            id: grooveFill
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.margins: 5
            width: styleData.handlePosition
            height:6
            color: sysPalette.highlight
            radius: 3
        }
    }

    handle: Item {
        property color borderColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.75)
        implicitWidth: 20
        implicitHeight: 20
        Rectangle {
            visible: control.enabled
            anchors.centerIn: parent
            height: 20
            width: 20
            radius: 10
            color: ColorUtils.adjustAlpha(sysPalette.shadow, 0.15)
            transform: Translate { x: 0; y: 0 }
        }

        Rectangle {
            id: sHandle
            anchors.centerIn: parent
            height: 20
            width: 20
            radius: 10
            border.width: control.activeFocus ? 2 : 1
            border.color: (control.activeFocus || control.hovered || control.pressed) ? sysPalette.highlight : borderColor
            color: sysPalette.button
            transform: Translate {x: control.pressed ? 0 : -1; y: control.pressed ? 0 : -1}
        }
    }
}
