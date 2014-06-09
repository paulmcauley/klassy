/********************************************************************
Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.2
import QtGraphicalEffects 1.0
import org.kde.kwin.decoration 0.1

Decoration {
    function readConfig() {
        switch (decoration.readConfig("BorderSize", DecorationOptions.BorderNormal)) {
        case DecorationOptions.BorderTiny:
            borders.setBorders(3);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderLarge:
            borders.setBorders(8);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderVeryLarge:
            borders.setBorders(12);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderHuge:
            borders.setBorders(18);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderVeryHuge:
            borders.setBorders(27);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderOversized:
            borders.setBorders(40);
            extendedBorders.setAllBorders(0);
            break;
        case DecorationOptions.BorderNoSides:
            borders.setBorders(4);
            borders.setSideBorders(1);
            extendedBorders.setSideBorders(3);
            break;
        case DecorationOptions.BorderNone:
            borders.setBorders(1);
            extendedBorders.setBorders(3);
            break;
        case DecorationOptions.BorderNormal: // fall through to default
        default:
            borders.setBorders(4);
            extendedBorders.setAllBorders(0);
            break;
        }
        switch (decoration.readConfig("ButtonSize", DecorationOptions.BorderNormal)) {
        case DecorationOptions.BorderTiny:
            buttonSize = 18;
            break;
        case DecorationOptions.BorderLarge:
            buttonSize = 24;
            break;
        case DecorationOptions.BorderVeryLarge:
            buttonSize = 26;
            break;
        case DecorationOptions.BorderHuge:
            buttonSize = 28;
            break;
        case DecorationOptions.BorderVeryHuge:
            buttonSize = 30;
            break;
        case DecorationOptions.BorderOversized:
            buttonSize = 32;
            break;
        case DecorationOptions.BorderNormal: // fall through to default
        default:
            buttonSize = 22;
            break;
        }
        var titleAlignLeft = decoration.readConfig("titleAlignLeft", true);
        var titleAlignCenter = decoration.readConfig("titleAlignCenter", false);
        var titleAlignRight = decoration.readConfig("titleAlignRight", false);
        if (titleAlignRight) {
            root.titleAlignment = Text.AlignRight;
        } else if (titleAlignCenter) {
            root.titleAlignment = Text.AlignHCenter;
        } else {
            if (!titleAlignLeft) {
                console.log("Error reading title alignment: all alignment options are false");
            }
            root.titleAlignment = Text.AlignLeft;
        }
        root.showActiveHighlight = decoration.readConfig("showActiveHighlight", true);
        root.animateButtons = decoration.readConfig("animateButtons", false);
        root.titleShadow = decoration.readConfig("titleShadow", false);
        if (decoration.animationsSupported) {
            root.animationDuration = 150;
            root.animateButtons = false;
        }
        borders.setTitle(top.height);
        maximizedBorders.setTitle(top.height);
        padding.top = 10;
        padding.left = 10;
        padding.right = 20;
        padding.bottom = 20;
        titleRow.height = Math.max(root.buttonSize, caption.implicitHeight);
        top.height = titleRow.anchors.topMargin +
                titleRow.height +
                4 +
                titleBarSpacer.height;
    }
    ColorHelper {
        id: colorHelper
    }
    DecorationOptions {
        id: options
        deco: decoration
    }
    property alias buttonSize: titleRow.height
    property alias titleAlignment: caption.horizontalAlignment
    property color titleBarColor: options.titleBarColor
    property color windowBackgroundColor: options.borderColor
    // set by readConfig after Component completed, ensures that buttons do not flicker
    property int animationDuration: 200
    property bool animateButtons: true
    property bool titleShadow: true
    property bool showActiveHighlight: true
    Behavior on titleBarColor {
        ColorAnimation {
            duration: root.animationDuration
        }
    }
    id: root
    alpha: false
    padding: {
        top: 10
        left: 10
        right: 20
        bottom: 20
    }

    Rectangle {
        SystemPalette { id: sysPal; colorGroup: SystemPalette.Active }
        id: paddingRect
        color: "transparent"
        anchors.fill: parent

        ShadowFrame {
            anchors.fill: parent
            anchors.rightMargin: decoration.active ? 0 : 6
            anchors.bottomMargin: decoration.active ? 0 : 6
        }

        Rectangle {
            id: windowRect
            color: root.windowBackgroundColor
            anchors {
                fill: parent
                topMargin: decoration.maximized ? 0 : root.padding.top
                leftMargin: decoration.maximized ? 0 : root.padding.left
                rightMargin: decoration.maximized ? 0 : root.padding.right
                bottomMargin: decoration.maximized ? 0 : root.padding.bottom
            }
            radius: 3

            Rectangle {
                id: borderLeft
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                visible: !decoration.maximized
                width: root.borders.left
                color: "transparent"
            }
            Rectangle {
                id: borderRight
                anchors {
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
                visible: !decoration.maximzied
                width: root.borders.right
                color: "transparent"
            }
            Rectangle {
                id: borderBottom
                anchors {
                    left: parent.right
                    right: parent.left
                    bottom: parent.bottom
                }
                height: root.borders.bottom
                visible: !decoration.maximzied
                color: "transparent"
            }

            Rectangle {
                id: top
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    topMargin: decoration.maximized ? -1*titleRow.anchors.topMargin : 0
                }
                height: titleRow.anchors.topMargin +
                        titleRow.height +
                        4 +
                        titleBarSpacer.height
                radius: decoration.maximized ? 0 : 3
                gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: decoration.active ? Qt.lighter(options.titleBarColor, 1.2) :
                                                       Qt.lighter(options.titleBarColor, 1.0)
                        }
                        GradientStop {
                            position: 0.8
                            color: options.titleBarColor
                        }
                        GradientStop {
                            position: 1.0
                            color: options.titleBarColor
                        }
                    }
                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                        bottomMargin: titleBarSpacer.height
                    }
                    height:3
                    color: options.titleBarColor
                }
                Rectangle {
                    id: activeHighlight
                    visible: root.showActiveHighlight
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                        bottomMargin: titleBarSpacer.height
                    }
                    height: 1
                    color: decoration.active ? sysPal.highlight : sysPal.window
                }
                Rectangle {
                    id: titleBarSpacer
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    height: 3
                    color: sysPal.window
                }

                MouseArea {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                    anchors.fill: parent
                    onDoubleClicked: decoration.titlebarDblClickOperation()
                    onPressed: {
                        if (mouse.button == Qt.LeftButton) {
                            mouse.accepted = false;
                        } else {
                            decoration.titlePressed(mouse.button, mouse.buttons);
                        }
                    }
                    onReleased: decoration.titleReleased(mouse.button, mouse.buttons)
                }

                Item {
                    id: titleRow
                    height: root.buttonSize
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        topMargin: 4
                        leftMargin: decoration.maximized ? 0 : 4
                        rightMargin: decoration.maximized ? 0 : 4
                    }
                    ButtonGroup {
                        id: leftButtonGroup
                        spacing: 2
                        explicitSpacer: root.buttonSize
                        menuButton: menuButtonComponent
                        appMenuButton: appMenuButtonComponent
                        minimizeButton: minimizeButtonComponent
                        maximizeButton: maximizeButtonComponent
                        keepBelowButton: keepBelowButtonComponent
                        keepAboveButton: keepAboveButtonComponent
                        helpButton: helpButtonComponent
                        shadeButton: shadeButtonComponent
                        allDesktopsButton: stickyButtonComponent
                        closeButton: closeButtonComponent
                        buttons: options.titleButtonsLeft
                        anchors {
                            top: parent.top
                            left: parent.left
                        }
                    }
                    Text {
                        id: caption
                        textFormat: Text.PlainText
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: leftButtonGroup.right
                            right: rightButtonGroup.left
                            rightMargin: 4
                            leftMargin: 4
                        }
                        color: options.fontColor
                        Behavior on color {
                            ColorAnimation { duration: root.animationDuration }
                        }
                        text: decoration.caption
                        style: root.titleShadow ? Text.Raised : Text.Normal
                        styleColor: colorHelper.shade(color, ColorHelper.ShadowShade)
                        font: options.titleFont
                        elide: Text.ElideMiddle
                    }
                    ButtonGroup {
                        id: rightButtonGroup
                        spacing: 2
                        explicitSpacer: root.buttonSize
                        menuButton: menuButtonComponent
                        appMenuButton: appMenuButtonComponent
                        minimizeButton: minimizeButtonComponent
                        maximizeButton: maximizeButtonComponent
                        keepBelowButton: keepBelowButtonComponent
                        keepAboveButton: keepAboveButtonComponent
                        helpButton: helpButtonComponent
                        shadeButton: shadeButtonComponent
                        allDesktopsButton: stickyButtonComponent
                        closeButton: closeButtonComponent
                        buttons: options.titleButtonsRight
                        anchors {
                            top: parent.top
                            right: parent.right
                        }
                    }
                }
            }

            Item {
                id: innerBorder
                anchors {
                    fill: parent
                    leftMargin: root.borders.left
                    rightMargin: root.borders.right
                    top: top.bottom
                    bottomMargin: root.borders.bottom
                }

                Rectangle {
                    anchors.fill: parent
                    visible: !decoration.maximized
                    color: "transparent"
                }
            }
        }
    }

    Component {
        id: maximizeButtonComponent
        BreezeMaximizeButton {
            buttonType: DecorationOptions.DecorationButtonMaximizeRestore
            size: root.buttonSize
        }
    }
    Component {
        id: keepBelowButtonComponent
        BreezeKeepBelowButton {
            buttonType: DecorationOptions.DecorationButtonKeepBelow
            size: root.buttonSize
        }
    }
    Component {
        id: keepAboveButtonComponent
        BreezeKeepAboveButton {
            buttonType: DecorationOptions.DecorationButtonKeepAbove
            size: root.buttonSize
        }
    }
    Component {
        id: helpButtonComponent
        BreezeHelpButton {
            buttonType: DecorationOptions.DecorationButtonQuickHelp
            size: root.buttonSize
        }
    }
    Component {
        id: minimizeButtonComponent
        BreezeMinimizeButton {
            buttonType: DecorationOptions.DecorationButtonMinimize
            size: root.buttonSize
        }
    }
    Component {
        id: shadeButtonComponent
        BreezeShadeButton {
            buttonType: DecorationOptions.DecorationButtonShade
            size: root.buttonSize
        }
    }
    Component {
        id: stickyButtonComponent
        BreezeStickyButton {
            buttonType: DecorationOptions.DecorationButtonOnAllDesktops
            size: root.buttonSize
        }
    }
    Component {
        id: closeButtonComponent
        BreezeCloseButton {
            buttonType: DecorationOptions.DecorationButtonClose
            size: root.buttonSize
        }
    }
    Component {
        id: menuButtonComponent
        MenuButton {
            width: root.buttonSize
            height: root.buttonSize
        }
    }
    Component {
        id: appMenuButtonComponent
        BreezeApplicationMenuButton {
            buttonType: DecorationOptions.DecorationButtonApplicationMenu
            size: root.buttonSize
        }
    }
    Component.onCompleted: {
        readConfig();
    }
    Connections {
        target: decoration
        onConfigChanged: {
            readConfig();
        }
    }
}
