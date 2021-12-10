# ClassiK

##### Table of Contents  
[Overview](#overview)  
[Installation](#installation)  
>[Pre-built packages](#prebuilt)  
>[Compile from source](#compile)

[System icon theme](#icons)   
[Donations](#donations)

&nbsp;

<a name="overview"/>

## Overview
_ClassiK_ (formerly _ClassikStyles_) is a highly customizable binary Window Decoration and Application Style plugin for recent versions of the KDE Plasma desktop. [Inspired by the classic style of KDE 1](https://forum.kde.org/viewtopic.php?f=285&t=138602?raw=true), _ClassiK_ is an attempt to update that classic style using a fork of KDE Breeze. Install with the instructions below, and then enable in System Settings -> Appearance -> Window Decorations, and also in System Setings -> Appearance -> Application Style.

![Screenshot of Button icons menu](screenshots/button_icon_menu.png?raw=true "Screenshot of Button icons menu")
* Also provides _Oxygen/Breeze_ and _Redmond_ button icon styles;
* Provides both a Window Decoration and an Application Style so that icons in dockable panels and MDI applications consistently match the titlebar icons (make sure to enable both in your Plasma Appearance settings!):
> [![Watch the demo video on enabling the Application Style](screenshots/video_dummy.png "Watch the demo video on enabling the Application Style")](http://paulmcauley.com/kde/classikstyles/ClassikStyles_ApplicationStyle_demo.mp4)

* Configurable button size, shape, translucency, colours and outlining

* Default buttons are rounded rectangles with translucent outlined accent colours designed to have a large lickable area and complement the "Blue Ocean" design refresh in Plasma 5.23:
> ![Default button style, dark](screenshots/highlight_gifs/cdark.gif?raw=true "Default button style, dark")![Default button style, light](screenshots/highlight_gifs/clight.gif?raw=true "Default button style, light")

* Configurable whether to draw bold button icons for HiDPI displays; by default automatically turns off bold icons on LoDPI 96DPI screens to prevent blurring;


* Full-height Rectangular or Circular button highlights:
> ![Square button highlight style, inheriting system highlight colours](screenshots/highlight_gifs/squareHighlight.gif?raw=true "Full-sized Rectangle button highlight style, inheriting system highlight colours")![Circle button highlight style, inheriting system highlight colours](screenshots/highlight_gifs/circleHighlight.gif?raw=true "Circle button highlight style, inheriting system highlight colours")
> 
> (Above-left configuration: Button highlight style: Full-sized Rectangle, Titlebar side margins: 0, Right-hand button spacing: 10
>
> Above-right configuration: Button highlight style: Small Circle, Titlebar side margins: 3, Right-hand button spacing: 5)

* Ability to inherit system colour-scheme highlight colours for hover and focus, as well as titlebar colour. Auto-enhances contrast in cases where it is poor;
* Option to have matching titlebar colour and border colour;

* Configurable button spacing (left and right), titlebar margins (sides, top/bottom and option for different maximized), corner radius; all with scaling for HiDPI:
> ![Screenshot of Alignment & Spacing tab](screenshots/alignment_and_spacing.png?raw=true "Screenshot of Alignment & Spacing tab")

* Transparency/opacity configurable for both active/inactive titlebars; setting to make maximized windows opaque; blur is configurable:
> ![Screenshot of Titlebar tab](screenshots/titlebar_tab.png?raw=true "Screenshot of Titlebar tab")

* Configurable thin window outline with optional accent colours:
> ![Composite screenshot of thin window outlines with Contrast blended with accent colour (dark)](screenshots/thin_window_outline_dark_contrast_accent.png?raw=true "Composite screenshot of thin window outlines with Contrast blended with accent colour (dark)")
> ![Composite screenshot of thin window outlines with Contrast blended with accent colour (light)](screenshots/thin_window_outline_light_contrast_accent.png?raw=true "Composite screenshot of thin window outlines with Contrast blended with accent colour (light)")

* Configurable scrollbars:
> ![Screenshot of scrollbar customization](screenshots/configurable_scrollbars.png?raw=true "Screenshot of scrollbar customization")

* Configurable animations

### Known issues
* GTK apps' titlebar buttons will not always update if you change a _ClassiK_ setting. Workaround: you need to select another window decoration and then select _ClassiK_ again to generate the GTK titlebar buttons; alternatively you can change your colour scheme and the GTK titlebar buttons will generate.

&nbsp;

<a name="installation"/>

## Installation
_ClassiK_ from version 3.0 onwards requires a recent version of KDE Plasma with __at least KDE Frameworks 5.86__, so please check you have this in kinfocenter before trying to install. You can either download the pre-built packages or compile from source.
<a name="prebuilt"/>

### Pre-built packages
[Respositories are available from the Open Build Service](https://software.opensuse.org//download.html?project=home%3Apaul4us&package=classikstyles) for OpenSUSE, Debian/KDE Neon, Fedora, Mageia and Arch/Manjaro to keep you on the latest version.
> NB: for OpenSUSE Leap you will [first need to install newer KDE packages](https://en.opensuse.org/SDB:KDE_repositories), otherwise you could corrupt your system!

&nbsp;

Raw binary packages are also available from the Open Build Service link above if you choose "Grab binary packages directly" -- a .rpm or .deb file from one of the selected distributions should also work on other distributions.

&nbsp;
&nbsp;
<a name="compile"/>

### Compile from source
### Step 1: First, Install Dependencies
#### OpenSUSE Tumbleweed/Leap build dependencies
(Leap requires newer KDE packages from https://en.opensuse.org/SDB:KDE_repositories first):
```
sudo zypper in cmake extra-cmake-modules libQt5Core-devel libQt5Gui-devel libQt5DBus-devel libqt5-qtx11extras-devel libkdecoration2-devel kcoreaddons-devel kguiaddons-devel kconfigwidgets-devel kwindowsystem-devel ki18n-devel kiconthemes-devel kpackage-devel libQt5QuickControls2-devel frameworkintegration-devel kcmutils-devel
```

#### Debian/Ubuntu/KDE Neon build dependencies
```
sudo apt install build-essential libkf5config-dev libkdecorations2-dev libqt5x11extras5-dev qtdeclarative5-dev extra-cmake-modules libkf5guiaddons-dev libkf5configwidgets-dev libkf5windowsystem-dev libkf5coreaddons-dev gettext cmake libkf5iconthemes-dev libkf5package-dev libkf5style-dev libkf5kcmutils-dev
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
Build and install from source script:
```
chmod +x install.sh
./install.sh
```

Uninstall build script:
```
chmod +x uninstall.sh
./uninstall.sh
```

&nbsp;
<a name="icons"/>

## System icon theme
To make your Plasma Desktop fully consistent, there are also matching system icon themes. These add consistent titlebar button icons to certain minor context menus in Plasma, for example, _Classik-with-Square-highlight_:
> <img src="https://raw.githubusercontent.com/paulmcauley/classikstyles/paulmcauley/selectable_buttoniconstyles/screenshots/classik_square_icons.png" alt="Screenshot of Classik with Square highlight icon theme" width="512">

To obtain these icon themes go to System Settings->Appearance->Icons->Get New Icons... and search for "classikstyles". Then download your theme of choice to match your Button icon style and Button highlight style:
> ![Screenshot of downloading matching ClassiK system icons](screenshots/icon_download.png?raw=true "Screenshot of downloading matching ClassiK system icons")

These icons inherit the _Breeze_ icon theme, only overriding the titlebar button icons. To inherit a different icon theme, go to ```~/.local/share/icons/``` and edit the ```Inherits``` line in your ```index.theme``` file.


<a name="donations"/>

## Donations
[![Donate using Liberapay](https://liberapay.com/assets/widgets/donate.svg "Donate using Liberapay")](https://liberapay.com/paulmcauley/donate)
or [![Donate using Paypal](https://www.paypalobjects.com/webstatic/en_US/i/buttons/PP_logo_h_100x26.png "Donate using Paypal")](https://www.paypal.com/donate?business=6N9RP4LDLNZCC&currency_code=GBP)
