import QtQuick 2.0
import QtQuick.Controls.Styles 1.1

ToolBarStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    padding {
        left: 4
        right: 4
        top: 4
        bottom: 4
    }
    background: Rectangle {
        implicitHeight: 16
        implicitWidth: 200
        color: sysPalette.window
    }
}
