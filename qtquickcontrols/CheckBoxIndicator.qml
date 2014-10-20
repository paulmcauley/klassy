import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0
import "ColorUtils.js" as ColorUtils

Item {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    property color enabledCheckedColor: sysPalette.highlight
    property color enabledNotCheckedColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.5)
    property color hoveredColor: ColorUtils.blendColors(sysPalette.highlight, sysPalette.window, 0.5)
    property color disabledColor: ColorUtils.blendColors(sysPalette.windowText, sysPalette.window, 0.6)
    opacity: control.enabled ? 1.0 : 0.5
    implicitHeight: 28
    implicitWidth: 28
    Rectangle {
        visible: control.enabled
        anchors.centerIn: parent
        implicitHeight: parent.height - 10
        implicitWidth: parent.width - 10
        radius: 2
        color: ColorUtils.adjustAlpha(sysPalette.shadow, 0.15)
        transform: Translate {x: 1; y: 1}
    }
    Rectangle {
        anchors.centerIn: parent
        implicitWidth: parent.width - 10
        implicitHeight: parent.height - 10
        border.width: 1
        border.color: control.enabled ? ((control.checked  || control.checkedState === Qt.PartiallyChecked) ? enabledCheckedColor : enabledNotCheckedColor) : disabledColor
        radius: 2
        color: sysPalette.window
        transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
    }
    Rectangle {
        visible: control.hovered
        anchors.centerIn: parent
        implicitWidth: parent.width - 10
        implicitHeight: parent.height - 10
        border.width: 1
        border.color: hoveredColor
        radius: 2
        color: sysPalette.window
        transform: Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0}
    }
    Rectangle {
        id: __checkMark
        anchors.centerIn: parent
        color: control.enabled ? enabledCheckedColor : disabledColor
        height: 12
        width: 12
        Rectangle {
            visible: (control.checked || control.checkedState === Qt.PartiallyChecked) & control.hovered
            anchors.centerIn: parent
            color: hoveredColor
            height: 12
            width: 12
        }
        Triangle {
            visible: control.checkedState === Qt.PartiallyChecked
            sideLength: 8
            color: sysPalette.window
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 2
            anchors.verticalCenterOffset: -1
        }
        transform: [
            Translate {x: control.pressed ? 1 : 0; y: control.pressed ? 1 : 0},
            Rotation {
                id: __checkRotator
                origin.x: 6
                origin.y: 6
                axis { x: 1; y: -1; z: 0 }
                angle: (control.checked || control.checkedState === Qt.PartiallyChecked) ? 0 : 270
            }
        ]
        PropertyAnimation {
            id: __checkFlipAnimation
            target: __checkRotator
            property: "angle"
            duration: 100
            from: 270 ; to: 360
            onStarted: __checkMark.visible = true
        }
        PropertyAnimation {
            id: __unCheckFlipAnimation
            target: __checkRotator
            property: "angle"
            duration: 100
            from: 360; to: 270
            onStopped: __checkMark.visible = false
        }
        Component.onCompleted: {
            control.checkedStateChanged.connect(flipChecked)
        }
        function flipChecked() {
            if (control.checked || control.checkedState === Qt.PartiallyChecked) {
                __checkFlipAnimation.running = true;
            } else {
                __unCheckFlipAnimation.running = true;
            }
        }

    }
}
