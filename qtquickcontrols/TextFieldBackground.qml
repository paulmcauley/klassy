import QtQuick 2.1
import "ColorUtils.js" as ColorUtils

Rectangle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }
    opacity: control.enabled ? 1.0 : 0.5
    implicitHeight: 32
    implicitWidth: 148
    border.width: control.activeFocus ? 2 : 1
    border.color: (control.activeFocus || control.hovered) ? sysPalette.highlight : ColorUtils.adjustAlpha(sysPalette.windowText, 0.3)
    radius: 2.5
    color: sysPalette.base
}
