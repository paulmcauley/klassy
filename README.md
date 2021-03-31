# ClassikStyles
## Overview
Fork of KDE Breeze to provide highly customizable _Classik_ and _Kite_ window decoration styles in a binary. Install with the instructions below, and then enable in System Settings -> Appearance -> Window Decorations, and also in System Setings -> Appearance -> Application Style.
* Also provides _Oxygen/Breeze_ and _Redmond_ button icon styles;
* Provides both a Window Decoration and an Application Style so that icons in dockable panels and MDI applications consistently match the titlebar icons (make sure to enable both in your Plasma Appearance settings!);
* Configurable whether to draw bold button icons for HiDPI displays; by default automatically turns off bold icons on LoDPI 96DPI screens to prevent blurring;
* Square or circular button highlights (square is now default as has a larger clickable area).
* Ability to inherit system colour-scheme highlight colours for hover and focus, as well as titlebar colour. Auto-enhances contrast in cases where it is poor;
* Option to have matching titlebar colour and border colour;
* Configurable button spacing (left and right), titlebar margins (sides, top/bottom and option for different maximized), all with scaling for HiDPI;
* Configurable corner radius, also scales with different DPIs;
* Transparency/opacity configurable for both active/inactive windows; setting to turn off opacity for maximized windows; blur is enabled;

### Known issues
* GTK apps' titlebar buttons will not update if you change a setting. Workaround: you need to select another window decoration or colour and then select ClassikStyles again to generate them.
* If you have _Bold button icons_ set to _Auto_, GTK apps' titlebar buttons will be rendered as _Fine_. Workaround: explicitly select _Bold_ if you want bold.

## Installation
### Pre-built packages
OpenSUSE Tumbleweed/Leap repository:
> https://software.opensuse.org//download.html?project=home%3Apaul4us&package=classikstyles

> NB: for Leap you will first need to install newer KDE packages from https://en.opensuse.org/SDB:KDE_repositories

&nbsp;
&nbsp;

Raw RPM binary packages are also available at https://github.com/paulmcauley/classikstyles/releases/tag/2.0.breeze5.21.80 -- these RPMs will also work in other RPM-based distributions such as Fedora or Mageia.

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
