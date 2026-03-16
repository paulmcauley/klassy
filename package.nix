{ lib, stdenv, fetchFromGitHub, cmake, kdePackages, }:

stdenv.mkDerivation rec {
  pname = "klassy";
  version = "6.5.3";

  src = ./.;

  nativeBuildInputs =
    [ cmake kdePackages.extra-cmake-modules kdePackages.wrapQtAppsHook ];

  buildInputs = with kdePackages; [
    qtbase
    qtdeclarative
    qtsvg
    frameworkintegration
    kcmutils
    kcolorscheme
    kconfig
    kconfigwidgets
    kcoreaddons
    kguiaddons
    ki18n
    kiconthemes
    kirigami
    kservice
    kwidgetsaddons
    kwindowsystem
    kdecoration
    libplasma
  ];

  dontWrapQtApps = true;

  cmakeFlags = [
    "-DBUILD_QT5=OFF"
    "-DBUILD_QT6=ON"
    "-DBUILD_TESTING=OFF"
    "-DKDE_INSTALL_USE_QT_SYS_PATHS=ON"
  ];

  meta = {
    description = "Highly customizable window decoration for KDE Plasma";
    homepage = "https://github.com/paulmcauley/klassy";
    license = with lib.licenses; [
      bsd3
      cc0
      gpl2Only
      gpl2Plus
      gpl3Only
      gpl3Plus
      mit
    ];
    platforms = lib.platforms.linux;
  };
}
