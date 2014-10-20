import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

SpinBoxStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    background: TextFieldBackground{}

    incrementControl: ScrollArrow {
        arrowColor: (styleData.upHovered && styleData.upEnabled) ? sysPalette.highlight : sysPalette.text
        opacity: control.enabled ? 1.0 : 0.5
        transform: Translate {x: styleData.upPressed ? 1 : 0; y: styleData.upPressed ? 2 : 1}
    }
    decrementControl: ScrollArrow {
        arrowColor: (styleData.downHovered && styleData.downEnabled) ? sysPalette.highlight : sysPalette.text
        rotation: 180
        opacity: control.enabled ? 1.0 : 0.5
        transform: Translate {x: styleData.downPressed ? 1 : 0; y: styleData.downPressed ? 0 : -1}
    }
}
