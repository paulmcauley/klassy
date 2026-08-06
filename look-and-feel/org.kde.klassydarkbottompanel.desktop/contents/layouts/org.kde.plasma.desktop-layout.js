loadTemplate("org.kde.klassy.plasma.desktop.bottomPanel");

const kwinConfig = ConfigFile('kwinrc');

kwinConfig.group = 'Effect-overview';
kwinConfig.writeEntry('BorderActivate', '9'); //disable overview

kwinConfig.group = 'Effect-windowview';
kwinConfig.writeEntry('BorderActivateAll', '3'); //overview all desktops bottom-right
