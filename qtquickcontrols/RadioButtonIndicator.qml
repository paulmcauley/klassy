import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import "ColorUtils.js" as ColorUtils


Item {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }
    property color enabledCheckedColor: sysPalette.highlight
    property color enabledNotCheckedColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.5)
    property color hoveredColor: ColorUtils.blendColors(sysPalette.highlight, sysPalette.window, 0.5)
    property color disabledColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.4)
    opacity: control.enabled ? 1.0 : 0.5
    width: 28
    height: 28

    Rectangle {
        visible: control.enabled
        anchors.centerIn: parent
        height: 18
        width: 18
        radius: 11
        color: ColorUtils.adjustAlpha(sysPalette.shadow, 0.15)
        transform: Translate { x: 1; y: 1 }
    }
    Rectangle {
        anchors.centerIn: parent
        height: 18
        width: 18
        radius: 10
        color: sysPalette.window
        border {
            width: 1
            color: control.checked ? (control.enabled ? enabledCheckedColor : disabledColor) : enabledNotCheckedColor
        }
        transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
    }
    Rectangle {
        visible: control.hovered
        anchors.centerIn: parent
        height: 18
        width: 18
        radius: 10
        color: sysPalette.window
        border {
            width: 2
            color: hoveredColor
        }
        transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
    }
    Rectangle {
        id: __rBMark
        anchors.centerIn: parent
        height: 12
        width: 12
        radius: 6
        color: control.enabled ? (control.checked ? enabledCheckedColor : enabledNotCheckedColor) : disabledColor
        Rectangle {
            visible: control.hovered
            anchors.centerIn: parent
            height: 12
            width: 12
            radius: 6
            color: hoveredColor
        }
        transform: [
            Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0},
            Rotation {
                id: __rBRotator
                origin.x: 6
                origin.y: 6
                axis { x: 1; y: -1; z: 0 }
                angle: control.checked ? 0 : 270
            }
        ]
        PropertyAnimation {
            id: __rBFlipChecked
            target: __rBRotator
            property: "angle"
            duration: 100
            from: 270 ; to: 360
            onStarted: __rBMark.visible = true;
        }
        PropertyAnimation {
            id: __rBFlipUnChecked
            target: __rBRotator
            property: "angle"
            duration: 100
            from: 180 ; to: 270
            onStopped: __rBMark.visible = false;
        }
        Component.onCompleted: {
            control.checkedChanged.connect(flipChecked)
        }
        function flipChecked() {
            if (control.checked) {
                __rBFlipChecked.running = true;
            } else {
                __rBFlipUnChecked.running = true;
            }
        }
    }
}

