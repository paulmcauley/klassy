# classikstyles
## Overview
Fork of KDE Breeze to provide Classik and Kite window decoration styles in a binary.
* Also provides Oxygen/Breeze and Redmond button icon styles;
* Provides both a Window Decoration and an Application Style so that icons in dockable panels in applications match the titlebar icons;
* Configurable button spacing (left and right);
* Configurable whether to draw bold button icons for HiDPI displays.

## Installation
### Pre-built packages
OpenSUSE Tumbleweed repository:
https://software.opensuse.org/package/classikstyles

RPM package for OpenSUSE Tumbleweed/Leap 15.2 (Leap works with newer KDE packages from https://en.opensuse.org/SDB:KDE_repositories and may also work in other distributions):

https://download.opensuse.org/repositories/home:/paul4us/openSUSE_Tumbleweed/x86_64/classikstyles-1.0.breeze5.21.80-20.1.x86_64.rpm (x64)

https://download.opensuse.org/repositories/home:/paul4us/openSUSE_Tumbleweed/i586/classikstyles-1.0.breeze5.21.80-20.1.i586.rpm (x86)

https://download.opensuse.org/repositories/home:/paul4us/openSUSE_Factory_ARM/aarch64/classikstyles-1.0.breeze5.21.80-20.1.aarch64.rpm (ARM64)

https://download.opensuse.org/repositories/home:/paul4us/openSUSE_Factory_ARM/armv7hl/classikstyles-1.0.breeze5.21.80-20.1.armv7hl.rpm (ARM32)

### Compile from source
OpenSUSE Tumbleweed build dependencies:
sudo zypper in cmake extra-cmake-modules libQt5Core-devel libQt5Gui-devel libQt5DBus-devel libqt5-qtx11extras-devel libkdecoration2-devel kcoreaddons-devel kguiaddons-devel kconfigwidgets-devel kwindowsystem-devel ki18n-devel kiconthemes-devel kpackage-devel libQt5QuickControls2-devel frameworkintegration-devel kcmutils-devel

Build and install from source script:
chmod +x install.sh
./install.sh

Uninstall build script:
chmod +x uninstallsh
./uninstall.sh
