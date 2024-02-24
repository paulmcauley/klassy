loadTemplate("org.kde.klassy.plasma.desktop.leftPanel");

const kwinConfig = ConfigFile('kwinrc');
kwinConfig.group = 'Effect-windowview';
kwinConfig.writeEntry('BorderActivateAll', '5'); //present windows all desktops bottom-left
