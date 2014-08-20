import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

ScrollViewStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    frame: Rectangle {
        color:"transparent"
    }
    scrollBarBackground: ScrollBar {
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
    minimumHandleLength: 32
    incrementControl: ScrollArrow {
        arrowColor: styleData.hovered ? sysPalette.highlight : sysPalette.text
        rotation: styleData.horizontal ? 90 : 180
        transform: Translate {x: styleData.pressed ? 1 : 0; y: styleData.pressed ? 1 : 0}
    }
    decrementControl: ScrollArrow {
        arrowColor: styleData.hovered ? sysPalette.highlight : sysPalette.text
        rotation: styleData.horizontal ? -90 : 0
        transform: Translate {x: styleData.pressed ? 1 : 0; y: styleData.pressed ? 1 : 0}
    }
}
