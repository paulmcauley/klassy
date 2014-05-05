import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

RadioButtonStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    indicator: RadioButtonIndicator{}

    label: Rectangle {
        opacity: control.enabled ? 1.0 : 0.5
        implicitWidth: rBText.implicitWidth + 4
        implicitHeight: rBText.implicitHeight + 4
        color: "transparent"

        Rectangle {
            visible: control.activeFocus
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 1
            color: sysPalette.highlight
        }

        Text {
            id:rBText
            anchors.centerIn: parent
            text: control.text
            color: sysPalette.windowText
        }
    }

}
