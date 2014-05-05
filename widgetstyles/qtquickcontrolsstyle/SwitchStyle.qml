import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

SwitchStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    groove: Rectangle {
        opacity: control.enabled ? 1.0 : 0.5
        implicitWidth: 48
        implicitHeight: 24
        radius: 12
        color: control.checked ? ColorUtils.adjustAlpha(sysPalette.highlight, 1) : ColorUtils.adjustAlpha(sysPalette.buttonText, 0.3)
    }

    handle: Rectangle {
        implicitWidth: 24
        implicitHeight: 24
        width: 24
        height: 24
        color: "transparent"
        Rectangle {
            opacity: control.enabled ? 1.0 : 0.5
            anchors.centerIn: parent
            width: 22
            height: 22
            radius: 11
            color: sysPalette.window
            border.width: control.activeFocus ? 2 : 1
            border.color: (control.activeFocus || control.hovered) ? sysPalette.highlight : ColorUtils.adjustAlpha(sysPalette.buttonText, 0.1)
        }
    }
}
