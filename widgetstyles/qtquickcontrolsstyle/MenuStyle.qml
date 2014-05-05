import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0
import "ColorUtils.js" as ColorUtils

Style {
    id: styleRoot

    property var __syspal: SystemPalette {
        colorGroup: control.enabled ?
                        SystemPalette.Active : SystemPalette.Disabled
    }
    property string __menuItemType: "menuitem"
    property real maxPopupHeight: 600 // ### FIXME Screen.desktopAvailableHeight * 0.99

    property Component frame: Rectangle {
        width: (parent ? parent.contentWidth : 0) + 2
        height: (parent ? parent.contentHeight : 0) + 2

        color: __syspal.window
        border { width: 1; color: __syspal.highlight }
        radius: 2

        property int subMenuOverlap: -1
        property real maxHeight: maxPopupHeight
        property int margin: 1
    }

    property Component menuItem: Rectangle {
        x: 1
        y: 1
        implicitWidth: Math.max((parent ? parent.width : 0),
                                18 + text.paintedWidth + (rightDecoration.visible ? rightDecoration.width + 40 : 12))
        implicitHeight: isSeparator ? text.font.pixelSize / 2 : !!scrollerDirection ? text.font.pixelSize * 0.75 : text.paintedHeight + 8
        color: selected && enabled ? syspal.highlight: backgroundColor
        border.width: 1
        border.color: selected && enabled ? Qt.darker(selectedColor, 1) : color
        readonly property int leftMargin: __menuItemType === "menuitem" ? 18 : 0

        readonly property color backgroundColor: syspal.window
        readonly property color selectedColor: syspal.highlight
        antialiasing: true

        SystemPalette {
            id: syspal
            colorGroup: enabled ? SystemPalette.Active : SystemPalette.Disabled
        }

        readonly property string itemText: parent ? parent.text : ""
        readonly property bool mirrored: Qt.application.layoutDirection === Qt.RightToLeft

        Loader {
            id: checkMark
            x: mirrored ? parent.width - width - 4 : 4
            y: 6
            active: __menuItemType === "menuitem" && !!menuItem && !!menuItem["checkable"]
            sourceComponent: exclusive ? exclusiveCheckMark : nonExclusiveCheckMark

            readonly property bool checked: !!menuItem && !!menuItem.checked
            readonly property bool exclusive: !!menuItem && !!menuItem["exclusiveGroup"]

            Component {
                id: nonExclusiveCheckMark
                Rectangle {
                    x: 1
                    width: 12
                    height: 12
                    color: syspal.window
                    border.color: checkMark.checked ? syspal.highlight : syspal.windowText
                    antialiasing: true

                    Rectangle {
                        antialiasing: true
                        visible: checkMark.checked
                        color: syspal.highlight
                        radius: 1
                        anchors.margins: 4
                        anchors.fill: parent
                    }
                }
            }

            Component {
                id: exclusiveCheckMark
                Rectangle {
                    x: 1
                    width: 12
                    height: 12
                    color: syspal.window
                    border.color: checkMark.checked ? syspal.highlight : syspal.windowText
                    antialiasing: true
                    radius: height/2

                    Rectangle {
                        anchors.centerIn: parent
                        visible: checkMark.checked
                        anchors.margins: 4
                        anchors.fill: parent
                        color: syspal.highlight
                        antialiasing: true
                        radius: 4
                    }
                }
            }
        }

        Text {
            id: text
            visible: !isSeparator
            text: StyleHelpers.stylizeMnemonics(itemText)
            readonly property real offset: __menuItemType === "menuitem" ? 24 : 6
            x: mirrored ? parent.width - width - offset : offset
            anchors.verticalCenter: parent.verticalCenter
            renderType: Text.NativeRendering
            color: selected && enabled ? syspal.highlightedText : syspal.windowText
        }

        Text {
            id: rightDecoration
            readonly property string shortcut: !!menuItem && menuItem["shortcut"] || ""
            visible: isSubmenu || shortcut !== ""
            text: isSubmenu ? mirrored ? "\u25c2" : "\u25b8" // BLACK LEFT/RIGHT-POINTING SMALL TRIANGLE
                            : shortcut
            LayoutMirroring.enabled: mirrored
            anchors {
                right: parent.right
                rightMargin: 6
                baseline: isSubmenu ? undefined : text.baseline
            }
            font.pixelSize: isSubmenu ? text.font.pixelSize : text.font.pixelSize * 0.9
            color: text.color
            renderType: Text.NativeRendering
            style: selected || !isSubmenu ? Text.Normal : Text.Raised; styleColor: Qt.lighter(color, 4)
        }

        ScrollArrow {
            id: scrollerDecoration
            visible: !!scrollerDirection
            anchors.centerIn: parent
            implicitWidth: 10
            implicitHeight: 6
            rotation: scrollerDirection === "up" ? 180 : 0
            arrowColor: syspal.buttonText
        }

        Rectangle {
            visible: isSeparator
            width: parent.width - 2
            height: 1
            x: 1
            anchors.verticalCenter: parent.verticalCenter
            color: ColorUtils.adjustAlpha(syspal.windowText, 0.2)
        }
    }

    property Component scrollerStyle: Style {
        padding { left: 0; right: 0; top: 0; bottom: 0 }
        property bool scrollToClickedPosition: false
        property Component frame: Item { visible: false }
        property Component corner: Item { visible: false }
        property Component __scrollbar: Item { visible: false }
        property bool useScrollers: true
    }
}
