import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

GridLayout {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    anchors.fill: parent
    anchors.margins: 16
    Layout.minimumWidth: 400
    Layout.minimumHeight: 300
    columns: 3
    rows: 3
    rowSpacing: 16
    columnSpacing: 8


    ComboBox {
        Layout.column: 0
        Layout.row: 0
        Layout.alignment: Qt.AlignHCenter
        style: ComboBoxStyle{}
        currentIndex: 0
        model: [ "ComboBox", "Apple", "Coconut" ]
    }

    ComboBox {
        enabled: false
        Layout.column: 0
        Layout.row: 1
        Layout.alignment: Qt.AlignHCenter
        style: ComboBoxStyle{}
        currentIndex: 0
        model: [ "ComboBox", "Apple", "Coconut" ]
    }

    ComboBox {
        Layout.column: 1
        Layout.row: 0
        Layout.alignment: Qt.AlignHCenter
        style: ComboBoxStyle{}
        currentIndex: 0
        editable: true
        model: [ "Editable ComboBox", "Apple", "Coconut" ]
    }

    SpinBox {
        Layout.column: 1
        Layout.row: 1
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumWidth: 148
        style: SpinBoxStyle{}
        minimumValue: 0
        maximumValue: 100
        value: 42
    }

    TextField {
        Layout.column: 2
        Layout.row: 0
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumWidth: 148
        style: TextFieldStyle{}
        text: "Text field"
    }

    TextField {
        enabled: false
        Layout.column: 2
        Layout.row: 1
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumWidth: 148
        style: TextFieldStyle{}
        text: "Disabled text field"
    }

    ToolButton {
        anchors.margins: 8
        Layout.column: 0
        Layout.row: 2
        Layout.alignment: Qt.AlignHCenter
        style: ToolButtonStyle{}
        text: "Tool Button"
        iconName: "edit-cut"
//        checked: true
//        onClicked: checked = !checked;
    }

    RowLayout {
        Layout.column: 1
        Layout.row: 2
        Layout.alignment: Qt.AlignHCenter
        Switch {
            checked: true
            style: SwitchStyle{}
        }

        Label {
            text: "Switch"
        }
    }

    RowLayout {
        Layout.column: 2
        Layout.row: 2
        Layout.alignment: Qt.AlignHCenter

        BusyIndicator {
            anchors.margins: 8
            style:BusyIndicatorStyle{}
        }

        Label {
            text: "Busy Indicator"
        }
    }

 }
