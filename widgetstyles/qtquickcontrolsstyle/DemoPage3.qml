import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

GridLayout {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    anchors.fill: parent
    anchors.margins: 16
    Layout.minimumWidth: 400
    Layout.minimumHeight: 300
    columns: 2
    rows: 1
    rowSpacing: 16
    columnSpacing: 8

    ScrollView {
        Layout.column: 0
        Layout.row: 0
        Layout.alignment: Qt.AlignHCenter
        style: ScrollViewStyle{}

        ListView {
            model: contactModel
            delegate: contactDelegate
            Component {
                id: contactDelegate
                Item {
                    width: 180; height: 40
                    Column {
                        Text { text: '<b>Name:</b> ' + name }
                        Text { text: '<b>Number:</b> ' + number }
                    }
                }
            }

            ListModel {
                id: contactModel
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Bill Smith"
                    number: "555 3264"
                }
                ListElement {
                    name: "John Brown"
                    number: "555 8426"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
                ListElement {
                    name: "Sam Wise"
                    number: "555 0473"
                }
            }
        }
    }
    TextArea {
        height: 50
        width: parent.width/2
        Layout.alignment: Qt.AlignHCenter
        Layout.column: 1
        Layout.row: 0
        textMargin: 8
        style: TextAreaStyle{}
        text: "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
    }

}
