import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

TextAreaStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    frame: Rectangle {
        opacity: control.enabled ? 1.0 : 0.5
        implicitHeight: 300
        implicitWidth: 300
        border.width: control.activeFocus ? 2 : 1
        border.color: (control.activeFocus || control.hovered) ? sysPalette.highlight : ColorUtils.adjustAlpha(sysPalette.windowText, 0.3)
        radius: 2.5
        color: sysPalette.base
    }
    scrollBarBackground: ScrollBar {
        color: sysPalette.base
        anchors.centerIn: parent
        barColor: ColorUtils.adjustAlpha(sysPalette.windowText, 0.3)
    }
    handle: ScrollBar {
        barColor: styleData.hovered ? sysPalette.button :
                                      control.activeFocus ? sysPalette.highlight :
                                                            ColorUtils.adjustAlpha(sysPalette.text, 0.5)
        barBorder.color: sysPalette.highlight
        barBorder.width: styleData.hovered ? 2 : 0
    }
    incrementControl: Rectangle {
        implicitWidth: __scrollDownArrow.implicitWidth
        implicitHeight: __scrollDownArrow.implicitHeight
        color: sysPalette.base
        ScrollArrow {
            id: __scrollDownArrow
            arrowColor: styleData.hovered ? sysPalette.highlight : sysPalette.text
            rotation: styleData.horizontal ? 90 : 180
            transform: Translate {x: styleData.pressed ? 1 : 0; y: styleData.pressed ? 1 : 0}
        }
    }

    decrementControl: Rectangle {
        implicitWidth: __scrollUpArrow.implicitWidth
        implicitHeight: __scrollUpArrow.implicitHeight
        color: sysPalette.base
        ScrollArrow {
            id: __scrollUpArrow
            arrowColor: styleData.hovered ? sysPalette.highlight : sysPalette.text
            rotation: styleData.horizontal ? -90 : 0
            transform: Translate {x: styleData.pressed ? 1 : 0; y: styleData.pressed ? 1 : 0}
        }
    }
}
