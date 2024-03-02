const panel = new Panel;
panel.location = "left";
panel.height = Math.round(gridUnit * 10 / 3); //gridUnit is 18
panel.hiding = "normal";
panel.offset = 0;
panel.floating = false;

panel.addWidget("org.kde.plasma.kickoff");
panel.addWidget("org.kde.plasma.pager");
var taskmanager = panel.addWidget("org.kde.plasma.icontasks");
taskmanager.currentConfigGroup = ["General"];
taskmanager.writeConfig('launchers', 'preferred://filemanager,preferred://browser');

/* Next up is determining whether to add the Input Method Panel
 * widget to the panel or not. This is done based on whether
 * the system locale's language id is a member of the following
 * white list of languages which are known to pull in one of
 * our supported IME backends when chosen during installation
 * of common distributions. */

var langIds = ["as",    // Assamese
               "bn",    // Bengali
               "bo",    // Tibetan
               "brx",   // Bodo
               "doi",   // Dogri
               "gu",    // Gujarati
               "hi",    // Hindi
               "ja",    // Japanese
               "kn",    // Kannada
               "ko",    // Korean
               "kok",   // Konkani
               "ks",    // Kashmiri
               "lep",   // Lepcha
               "mai",   // Maithili
               "ml",    // Malayalam
               "mni",   // Manipuri
               "mr",    // Marathi
               "ne",    // Nepali
               "or",    // Odia
               "pa",    // Punjabi
               "sa",    // Sanskrit
               "sat",   // Santali
               "sd",    // Sindhi
               "si",    // Sinhala
               "ta",    // Tamil
               "te",    // Telugu
               "th",    // Thai
               "ur",    // Urdu
               "vi",    // Vietnamese
               "zh_CN", // Simplified Chinese
               "zh_TW"] // Traditional Chinese

if (langIds.indexOf(languageId) != -1) {
    panel.addWidget("org.kde.plasma.kimpanel");
}

panel.addWidget("org.kde.plasma.systemtray");
var digitalclock = panel.addWidget("org.kde.plasma.digitalclock");
digitalclock.currentConfigGroup = ["Appearance"];
digitalclock.writeConfig("customDateFormat", "ddd-d");
digitalclock.writeConfig("dateFormat", "custom");
panel.addWidget("org.kde.plasma.showdesktop");

const kwinConfig = ConfigFile('kwinrc');
kwinConfig.group = 'Effect-windowview';
kwinConfig.writeEntry('BorderActivateAll', '5'); //present windows all desktops bottom-left
