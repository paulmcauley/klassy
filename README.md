# ClassikStyles
## Overview
Fork of KDE Breeze to provide Classik and Kite window decoration styles in a binary.
* Also provides Oxygen/Breeze and Redmond button icon styles;
* Provides both a Window Decoration and an Application Style so that icons in dockable panels and MDI applications match the titlebar icons;
* Configurable button spacing (left and right);
* Configurable whether to draw bold button icons for HiDPI displays.

## Installation
### Pre-built packages
OpenSUSE Tumbleweed/Leap repository:
> https://software.opensuse.org//download.html?project=home%3Apaul4us&package=classikstyles

> NB: for Leap you will first need to install newer KDE packages from https://en.opensuse.org/SDB:KDE_repositories

&nbsp;
&nbsp;

Raw RPM binary package from OpenSUSE Open Build Service - these RPM packages may also work in other distributions:

https://software.opensuse.org//download.html?project=home%3Apaul4us&package=classikstyles#directopenSUSE

&nbsp;
&nbsp;

### Compile from source
### Step 1: First, Install Dependencies
#### OpenSUSE Tumbleweed/Leap build dependencies
(Leap requires newer KDE packages from https://en.opensuse.org/SDB:KDE_repositories first):
```
sudo zypper in cmake extra-cmake-modules libQt5Core-devel libQt5Gui-devel libQt5DBus-devel libqt5-qtx11extras-devel libkdecoration2-devel kcoreaddons-devel kguiaddons-devel kconfigwidgets-devel kwindowsystem-devel ki18n-devel kiconthemes-devel kpackage-devel libQt5QuickControls2-devel frameworkintegration-devel kcmutils-devel
```

#### Ubuntu/KDE Neon build dependencies
```
sudo apt install build-essential libkf5config-dev libkdecorations2-dev libqt5x11extras5-dev qtdeclarative5-dev extra-cmake-modules libkf5guiaddons-dev libkf5configwidgets-dev libkf5windowsystem-dev libkf5coreaddons-dev gettext cmake libkf5iconthemes-dev libkf5package-dev
```

#### Arch/Manjaro build dependencies
```
sudo pacman -S kdecoration qt5-declarative qt5-x11extras cmake make gcc extra-cmake-modules
```

#### Fedora build dependencies
```
sudo dnf install cmake extra-cmake-modules
sudo dnf install "cmake(Qt5Core)" "cmake(Qt5Gui)" "cmake(Qt5DBus)" "cmake(Qt5X11Extras)" "cmake(KF5GuiAddons)" "cmake(KF5WindowSystem)" "cmake(KF5I18n)" "cmake(KDecoration2)" "cmake(KF5CoreAddons)" "cmake(KF5ConfigWidgets)" "cmake(KF5IconThemes)" "cmake(KF5Package)" "cmake(Qt5Quick)" "cmake(KF5FrameworkIntegration)" "cmake(KF5KCMUtils)"
```

### Step 2: Then, build and install
First, edit the ```install.sh``` file and check that the ```-DKDE_INSTALL_LIBDIR``` corresponds to your /usr/lib64 or /usr/lib directory (distribution dependent):

> For OpenSUSE/Fedora:
> ```
> -DKDE_INSTALL_LIBDIR=lib64
> ```
> For Ubuntu:
> ```
> -DKDE_INSTALL_LIBDIR=lib
> ```

Then build and install from source script:
```
chmod +x install.sh
./install.sh
```

Uninstall build script:
```
chmod +x uninstall.sh
./uninstall.sh
```
