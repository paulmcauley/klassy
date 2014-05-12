import QtQuick 2.0
import org.kde.kwin.decoration 0.1

DecorationButton {
    id: button
    property real size
    width: size
    height: size
    Rectangle {
        anchors {
            fill: parent
            margins: 2
        }
        radius: width/2.0
        color: button.pressed ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.2) :
                                button.hovered ? Qt.rgba(options.fontColor.r, options.fontColor.g, options.fontColor.b, 0.5) :
                                                 "transparent"
        Behavior on color {
            enabled: root.animateButtons
            ColorAnimation { duration: root.animationDuration }
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 2
            color: button.hovered ? options.titleBarColor : options.fontColor
            Behavior on color {
                enabled: root.animateButtons
                ColorAnimation { duration: root.animationDuration*2 }
            }
            Rectangle {
                id: stroke1
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.verticalCenter
                height: 2
                width: parent.width - 6
                color: button.hovered ? options.fontColor : options.titleBarColor
                Behavior on color {
                    enabled: root.animateButtons
                    ColorAnimation { duration: root.animationDuration*2 }
                }
            }
            Rectangle {
                id: stroke2
                anchors.bottom: stroke1.top
                anchors.bottomMargin: 2
                anchors.horizontalCenter: parent.verticalCenter
                height: 2
                width: parent.width - 4
                color: button.hovered ? options.fontColor : options.titleBarColor
                Behavior on color {
                    enabled: root.animateButtons
                    ColorAnimation { duration: root.animationDuration*2 }
                }
            }
            Rectangle {
                id: stroke3
                anchors.top: stroke1.bottom
                anchors.topMargin: 2
                anchors.horizontalCenter: parent.verticalCenter
                height: 2
                width: parent.width - 4
                color: button.hovered ? options.fontColor : options.titleBarColor
                Behavior on color {
                    enabled: root.animateButtons
                    ColorAnimation { duration: root.animationDuration*2 }
                }
            }
        }
    }
    Component.onCompleted: {
        if (buttonType == DecorationOptions.DecorationButtonApplicationMenu) {
            visible = decoration.appMenu;
        }
     }
     Connections {
         target: decoration
         onAppMenuChanged: {
             if (buttonType == DecorationOptions.DecorationButtonApplicationMenu) {
                 visible = decoration.appMenu;
             }
         }
     }

}
