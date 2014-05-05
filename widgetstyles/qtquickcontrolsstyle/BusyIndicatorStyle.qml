import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

BusyIndicatorStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    indicator: Item {
        implicitHeight: 32
        implicitWidth: 32
        Rectangle {
            anchors.centerIn: parent
            height: 24
            width: 24
            radius: 12
            border.width: 2
            border.color: sysPalette.highlight
            color: "transparent"
            transform: Rotation {
                origin.x: 12
                origin.y: 12
                axis { x: 1; y: 0; z: 0 }
                NumberAnimation on angle {
                    running: control.running
                    loops: Animation.Infinite
                    duration: 1000
                    from: 0 ; to: 360
                }
            }
        }
        Rectangle {
            anchors.centerIn: parent
            height: 12
            width: 12
            radius: 6
            border.width: 2
            border.color: sysPalette.highlight
            color: sysPalette.highlight
        }
        NumberAnimation on rotation {
            running: control.running
            loops: Animation.Infinite
            duration: 3000
            from: 0 ; to: 360
        }
    }
}
