import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

CheckBoxStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    indicator: CheckBoxIndicator{}

    label: Item {
        opacity: control.enabled ? 1.0 : 0.5
        implicitWidth: __checkBoxtext.implicitWidth + 4
        implicitHeight: __checkBoxtext.implicitHeight + 4
        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 1
            color: control.activeFocus ? sysPalette.highlight : "transparent"
        }
        Text {
            id: __checkBoxtext
            anchors.centerIn: parent
            text: control.text
            color: sysPalette.windowText
        }
    }
}
