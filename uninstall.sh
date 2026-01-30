#!/bin/sh
# Modified from https://github.com/kupiqu/SierraBreezeEnhanced

ORIGINAL_DIR=$(pwd)

confirm() {
    # call with a prompt string or use a default
    read -r -p "${1:-Are you sure? [y/N]} " response
    case "$response" in
        [yY][eE][sS]|[yY])
            true
            ;;
        *)
            false
            ;;
    esac
}

uninstall() {
    sudo make uninstall && echo "Uninstalled successfully!"
}

cleanup() {
    cd $ORIGINAL_DIR

    sudo rm -rf build
}

# Recreate install manifest and uninstall
install_and_uninstall() {
    sh install.sh &&
    cd build &&
    uninstall
    echo It is possible that some files are left over at the following locations
    echo Please remove any files containing klassy in their name
    echo - /usr/bin/
    echo - /usr/lib64/
    echo - /usr/lib64/cmake/
    echo - /usr/lib64/qt5/plugins/styles/
    echo - /usr/lib64/qt6/plugins/org.kde.kdecoration3/
    echo - /usr/lib64/qt6/plugins/org.kde.kdecoration3.kcm/
    echo - /usr/lib64/qt6/plugins/styles/
    echo - /usr/share/color-schemes/
    echo - /usr/share/icons/
    echo - /usr/share/icons/hicolor/scalable/apps/
    echo - /usr/share/kservices5/
    echo - /usr/share/kstyle/themes/
    echo - /usr/share/plasma/desktoptheme/
    echo - /usr/share/plasma/layout-templates/
    echo - /usr/share/plasma/look-and-feel/
}

if [ ! -d "build" ]; then
    # If no build folder is found
    confirm "No installation found, (re)install and uninstall? [y/n]" && install_and_uninstall
else
    if test -f "$ORIGINAL_DIR/build/install_manifest.txt"; then
        # Remove normally
        echo Found $ORIGINAL_DIR/build/install_manifest.txt
        cd build &&
        uninstall &&
        cleanup
    else
        # If no install manifest found
        echo Did not find $ORIGINAL_DIR/build/install_manifest.txt
        confirm "(re)install and uninstall? [y/n]" && install_and_uninstall && cleanup
    fi
fi
