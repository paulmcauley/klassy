import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

ButtonStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    background: Item {
        property color borderColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.75)
        opacity: control.enabled ? 1.0 : 0.5
        implicitHeight: 32
        implicitWidth: 96
        Rectangle {
            anchors.centerIn: parent
            implicitHeight: parent.height - 2
            implicitWidth: parent.width - 2
            radius: 2.5
            color: ColorUtils.adjustAlpha(sysPalette.shadow, 0.2)
            transform: Translate {x: 1; y: 1}
        }
        Rectangle {
            anchors.centerIn: parent
            implicitWidth: parent.width - 2
            implicitHeight: parent.height - 2
            border.width: control.activeFocus ? 0: 1
            border.color: (control.activeFocus || control.hovered) ? sysPalette.highlight : borderColor
            radius: 2.5
            color: control.pressed || control.activeFocus ? sysPalette.highlight : sysPalette.button
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: control.activeFocus ? Qt.lighter(sysPalette.highlight, 1.03) :
                                                 Qt.lighter(sysPalette.button, 1.01)
                }
                GradientStop {
                    position: 1.0
                    color: control.activeFocus ? Qt.darker(sysPalette.highlight, 1.10) :
                                                 Qt.darker(sysPalette.button, 1.03)
                }
            }
            transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
        }
    }

    label: Item {
        opacity: control.enabled ? 1.0 : 0.5
        implicitWidth: __buttonText.implicitWidth + 16
        implicitHeight: __buttonText.implicitHeight + 8
        Text {
            id:__buttonText
            anchors.centerIn: parent
            text: control.text
            color: control.activeFocus ? sysPalette.highlightedText : sysPalette.buttonText
        }
        transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
    }
}

