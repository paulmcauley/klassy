var panel = new Panel;
var panelScreen = panel.screen;
const geo = screenGeometry(panelScreen);
panel.remove();

const aspectRatio = geo.width / geo.height;
var leftPanel = false;

if (aspectRatio < (16/9 - 0.001)) { //tallscreen e.g. 16:10 - sensible aspect ratios make the bottom panel work well
    leftPanel = false;
} else if (aspectRatio < (21/9 - 0.001)) { //16:9 - not enough vertical space in this silly but trendy aspect ratio, so use a leftPanel
    leftPanel = true;
} else { //ultrawide - let these weirdos have a bottom panel as the left panel will have a large mouse distance
    leftPanel = false;
}

if(leftPanel){
    const klassyConfig = ConfigFile('klassy/klassyrc');
    klassyConfig.group = 'Global';
    klassyConfig.writeEntry('AutoPanelLocation', 'Left');
    
    const kwinConfig = ConfigFile('kwinrc');
    kwinConfig.group = 'Effect-windowview';
    kwinConfig.writeEntry('BorderActivateAll', '5'); //present windows all desktops bottom-left
    
    loadTemplate("org.kde.klassy.plasma.desktop.leftPanel");
} else {
    const klassyConfig = ConfigFile('klassy/klassyrc');
    klassyConfig.group = 'Global';
    klassyConfig.writeEntry('AutoPanelLocation', 'Bottom');
    
    const kwinConfig = ConfigFile('kwinrc');
    kwinConfig.group = 'Effect-windowview';
    kwinConfig.writeEntry('BorderActivateAll', '3'); //present windows all desktops bottom-right
    
    loadTemplate("org.kde.klassy.plasma.desktop.bottomPanel");
}
