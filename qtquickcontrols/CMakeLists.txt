FILE(GLOB themeFiles *.qml)
INSTALL(FILES ${themeFiles} DESTINATION ${QML_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
INSTALL(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
INSTALL(FILES ColorUtils.js DESTINATION ${QML_INSTALL_DIR}/QtQuick/Controls/Styles/Breeze)
