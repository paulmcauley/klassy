loadTemplate("org.kde.klassy.plasma.desktop.leftPanel");

const kwinConfig = ConfigFile('kwinrc');
kwinConfig.group = 'Effect-overview';
kwinConfig.writeEntry('BorderActivate', '5'); //overview all desktops bottom-left
