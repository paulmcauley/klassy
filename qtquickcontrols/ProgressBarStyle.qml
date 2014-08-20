import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils

ProgressBarStyle {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    background: Rectangle {
        opacity: control.enabled ? 1.0 : 0.5
        anchors.centerIn: parent
        implicitWidth: 300
        implicitHeight: 16
        color: "transparent"

        Rectangle {
            visible: !control.indeterminate
            anchors.fill: parent
            anchors.margins: 5
            color: ColorUtils.adjustAlpha(sysPalette.buttonText, 0.3)
            radius: 3
        }
    }

    progress: Rectangle {
        id: __progressRoot
        opacity: control.enabled ? 1.0 : 0.5
        anchors.fill: parent
        color: "transparent"
        Rectangle {
            anchors.fill: parent
            anchors.margins: 5
            color: sysPalette.highlight
            radius: 3
            clip: true
            Row {
                visible: control.indeterminate
                anchors.fill: parent
                anchors.leftMargin: -28
                Repeater {
                    model: 100
                    Item {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 28
                        height: 6
                        Rectangle {
                            anchors.centerIn: parent
                            width: 6
                            height: 14
                            radius: 0
                            color: ColorUtils.blendColors(sysPalette.highlight, sysPalette.window, 0.7)
                            transform: Rotation {
                                angle: 90
                                origin.x: 3
                                origin.y: 7
                            }
                        }
                    }
                }
                NumberAnimation on anchors.leftMargin {
                    from: -28
                    to: 1
                    duration: 800
                    loops: Animation.Infinite
                    running: control.indeterminate
                }
            }
        }
    }
}

