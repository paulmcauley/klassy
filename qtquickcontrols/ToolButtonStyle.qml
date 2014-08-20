import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0
import "ColorUtils.js" as ColorUtils

Style {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    readonly property ToolButton control: __control
    property Component panel: Item {
        id: styleitem
        opacity: control.enabled ? 1.0 : 0.5
        implicitWidth: (hasIcon ? 36 : Math.max(label.implicitWidth + 16, 36))
                                 + (arrow.visible ? 10 : 0)
        implicitHeight: hasIcon ? 36 : Math.max(label.implicitHeight, 36)

        readonly property bool hasIcon: icon.status === Image.Ready || icon.status === Image.Loading

        Rectangle {
            anchors.centerIn: parent
            implicitWidth: parent.width - 2
            implicitHeight: parent.height - 2
            border.width: 1
            border.color: (control.activeFocus || control.hovered) ? sysPalette.highlight : ColorUtils.adjustAlpha(sysPalette.windowText, 0.25)
            radius: 2.5
            color: control.pressed || control.checked ? sysPalette.highlight : sysPalette.button
            visible: control.hovered || control.activeFocus || control.checked
        }
        Item {
            anchors.left: parent.left
            anchors.right: arrow.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            clip: true
            Text {
                id: label
                visible: !hasIcon
                anchors.centerIn: parent
                text: StyleHelpers.stylizeMnemonics(control.text)
                color: control.pressed || control.checked  ? sysPalette.highlightedText : sysPalette.buttonText
            }
            Image {
                id: icon
                anchors.centerIn: parent
                source: control.iconSource
            }
        }
        ScrollArrow {
            id: arrow
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: visible ? 3 : 0
            implicitWidth: control.menu !== null ? 6: 0
            implicitHeight: 6
            visible: control.menu !== null
            rotation: control.pressed ? 0 : 180
            arrowColor: (control.activeFocus && !control.editable) ?  sysPalette.highlightedText : sysPalette.buttonText
        }
    }
}

