loadTemplate("org.kde.klassy.plasma.desktop.leftPanel");

const kwinConfig = ConfigFile('kwinrc');

kwinConfig.group = 'Effect-overview';
kwinConfig.writeEntry('BorderActivate', '9'); //disable overview

kwinConfig.group = 'Effect-windowview';
kwinConfig.writeEntry('BorderActivateAll', '5'); //overview all desktops bottom-left
