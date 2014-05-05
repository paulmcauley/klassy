import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

TableViewStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    padding {left: 1; top: 1; right: 1; bottom: 1}
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
        ScrollArrow {
            id: __scrollUpArrow
            arrowColor: styleData.hovered ? sysPalette.highlight : sysPalette.text
            rotation: styleData.horizontal ? -90 : 0
            transform: Translate {x: styleData.pressed ? 1 : 0; y: styleData.pressed ? 1 : 0}
        }
    }
    headerDelegate: Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 0
        implicitHeight: 28
        implicitWidth: 60
        color: styleData.pressed ? ColorUtils.blendColors(sysPalette.window, sysPalette.highlight, 0.2) :
                                   sysPalette.window
        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 8
            anchors.verticalCenter: parent.verticalCenter
            text: styleData.value
            color: sysPalette.text
        }
        Rectangle {
            width: parent.width
            height: 1
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.horizontalCenter: parent.horizontalCenter
            color: ColorUtils.adjustAlpha(sysPalette.windowText, 0.1)
        }
        Rectangle {
            width: 1
            height: parent.height
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.topMargin: 1
            anchors.bottomMargin: 1
            color: ColorUtils.adjustAlpha(sysPalette.windowText, 0.2)
        }
    }
    rowDelegate: Rectangle {
        implicitHeight: 28
        implicitWidth: 60
//        anchors.right: parent.right
//        anchors.rightMargin: 2
        height: 28
        color: styleData.selected ? sysPalette.highlight:
                                    styleData.alternate ? sysPalette.alternateBase : sysPalette.base
    }
}
