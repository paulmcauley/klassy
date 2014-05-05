import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0
import "ColorUtils.js" as ColorUtils


/*!
    \qmltype MenuBarStyle
    \internal
    \ingroup applicationwindowstyling
    \inqmlmodule QtQuick.Controls.Styles
*/

Style {
    SystemPalette { id: syspal }

    readonly property color __backgroundColor: syspal.window

    property Component frame: Rectangle {
        width: control.__contentItem.width
        height: contentHeight
        color: __backgroundColor
    }

    property Component menuItem: Rectangle {
        width: text.width + 12
        height: text.height + 8
        color: sunken ? syspal.highlight :__backgroundColor

        Text {
            id: text
            text: StyleHelpers.stylizeMnemonics(menuItem.title)
            anchors.centerIn: parent
            renderType: Text.NativeRendering
            color: sunken ? syspal.highlightedText : syspal.windowText
        }
    }
}
