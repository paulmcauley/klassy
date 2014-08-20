import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

GridLayout {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    anchors {
        fill: parent
        leftMargin: 16
        topMargin: 16
        rightMargin: 16
        bottomMargin: 70
    }
    columns: 3
    rows: 4
    rowSpacing: 8
    columnSpacing: 8

    ExclusiveGroup { id: group }
    RadioButton {
        Layout.column: 0
        Layout.row: 0
        Layout.minimumWidth: parent.width/3
        exclusiveGroup: group
        checked: true
        style: RadioButtonStyle{}
        text: "Radio Button"
    }
    RadioButton {
        Layout.column: 0
        Layout.row: 1
        exclusiveGroup: group
        checked: false
        style: RadioButtonStyle{}
        text: "Radio Button"
    }
    RadioButton {
        enabled: false
        Layout.column: 0
        Layout.row: 2
        checked: true
        style: RadioButtonStyle{}
        text: "Disabled Radio Button"
    }
    RadioButton {
        enabled: false
        Layout.column: 0
        Layout.row: 3
        checked: false
        style: RadioButtonStyle{}
        text: "Disabled Radio Button"
    }
    CheckBox {
        Layout.column: 1
        Layout.row: 0
        Layout.minimumWidth: parent.width/3
        checked: true
        style: CheckBoxStyle{}
        text: "Checkbox"
    }
    CheckBox {
        Layout.column: 1
        Layout.row: 1
        partiallyCheckedEnabled: true
        checked: false
        style: CheckBoxStyle{}
        text: "Checkbox"
    }
    CheckBox {
        enabled: false
        Layout.column: 1
        Layout.row: 2
        checked: true
        style: CheckBoxStyle{}
        text: "Disabled Checkbox"
    }
    CheckBox {
        enabled: false
        Layout.column: 1
        Layout.row: 3
        checked: false
        style: CheckBoxStyle{}
        text: "Disabled Checkbox"
    }
    ProgressBar {
        Layout.fillWidth: true
        Layout.column: 2
        Layout.row: 0
        Layout.alignment: Qt.AlignCenter
        anchors.margins: 16
        value: 75
        minimumValue: 0
        maximumValue: 100
        style: ProgressBarStyle{}
    }

    ProgressBar {
        Layout.fillWidth: true
        Layout.column: 2
        Layout.row: 1
        Layout.alignment: Qt.AlignCenter
        anchors.margins: 16
        indeterminate: true
        style: ProgressBarStyle{}
    }

    Slider {
        Layout.fillWidth: true
        Layout.column: 2
        Layout.row: 2
        anchors.margins: 8
        value: 55
        minimumValue: 0
        maximumValue: 100
        style: SliderStyle{}
    }
    Button {
        anchors.margins: 8
        Layout.column: 2
        Layout.row: 3
        Layout.fillWidth: true
        style: ButtonStyle{}
        text: activeFocus ? "Focused button" : "Button"
    }
}
