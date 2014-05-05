project(BreezeStyle)

EXEC_PROGRAM(qmake ARGS "-query QT_INSTALL_QML" OUTPUT_VARIABLE STYLE_INSTALL_DIR)

FILE(GLOB themeFiles *.qml)
INSTALL(FILES ${themeFiles} DESTINATION ${STYLE_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
INSTALL(FILES qmldir DESTINATION ${STYLE_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
INSTALL(FILES ColorUtils.js DESTINATION ${STYLE_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
