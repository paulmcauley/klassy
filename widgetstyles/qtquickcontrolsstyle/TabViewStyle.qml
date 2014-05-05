import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

TabViewStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    frameOverlap: 1
    tabOverlap: 1
    tabsAlignment: Qt.AlignHCenter
    tab: Rectangle {
        scale: control.tabPosition === Qt.TopEdge ? 1 : -1
        color: styleData.selected ? ColorUtils.blendColors(sysPalette.window, sysPalette.base, 0.3) :
                                    styleData.hovered ? ColorUtils.adjustAlpha(sysPalette.highlight, 0.2) : ColorUtils.adjustAlpha(sysPalette.windowText, 0.2)
        implicitWidth: Math.max(text.implicitWidth + 8, 80)
        implicitHeight: Math.max(text.implicitHeight + 8, 28)
        border.color: ColorUtils.blendColors(sysPalette.window, sysPalette.windowText, 0.25)
        border.width: styleData.selected ? 1 : 0
        radius: 2.5
        Text {
            id: text
            anchors.centerIn: parent
            text: styleData.title
            color: sysPalette.windowText
            scale: control.tabPosition === Qt.TopEdge ? 1 : -1
        }
        Rectangle {
            anchors.bottom: text.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: text.implicitWidth
            height: 1
            color: sysPalette.highlight
            visible: styleData.activeFocus && styleData.selected
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 2
            height: styleData.selected ? 2 : 1
            color: styleData.selected ? ColorUtils.blendColors(sysPalette.window, sysPalette.base, 0.3) : ColorUtils.blendColors(sysPalette.window, sysPalette.windowText, 0.25)
        }
    }

    frame: Item {
        Rectangle {
            implicitHeight: 300
            implicitWidth: parent.width
            anchors.fill: parent
            border.color: ColorUtils.blendColors(sysPalette.window, sysPalette.windowText, 0.25)
            border.width: 1
            color: ColorUtils.blendColors(sysPalette.window, sysPalette.base, 0.3)
            radius: 2.5
        }
    }
}
