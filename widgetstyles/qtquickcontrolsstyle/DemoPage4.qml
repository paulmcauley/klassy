import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

GridLayout {
    SystemPalette { id: sysPalette; colorGroup: SystemPalette.Active }

    anchors.fill: parent
    anchors.margins: 16
    Layout.minimumWidth: 400
    Layout.minimumHeight: 300
    columns: 1
    rows: 1

    TableView {
        Layout.column: 0
        Layout.row: 0
        anchors.fill: parent
        anchors.margins: 8
        style: TableViewStyle{}
        ListModel {
           id: libraryModel
           ListElement{ title: "A Masterpiece" ; author: "Gabriel" }
           ListElement{ title: "Brilliance"    ; author: "Jens" }
           ListElement{ title: "Outstanding"   ; author: "Frederik" }
           ListElement{ title: "Brilliance"    ; author: "Jens" }
           ListElement{ title: "Outstanding"   ; author: "Frederik" }
           ListElement{ title: "Brilliance"    ; author: "Jens" }
           ListElement{ title: "Outstanding"   ; author: "Frederik" }
           ListElement{ title: "Brilliance"    ; author: "Jens" }
           ListElement{ title: "Outstanding"   ; author: "Frederik" }
        }
        TableViewColumn{ role: "title"  ; title: "Title" ; width: 100 }
        TableViewColumn{ role: "author" ; title: "Author" ; width: 200 }
        model: libraryModel
    }
}
