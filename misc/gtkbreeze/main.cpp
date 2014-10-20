/*
 Copyright 2014 Jonathan Riddell <jriddell@ubuntu.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of 
 the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Wee program to be run at login by kconf_update
// checks if gtk theme is set
// if not or if it is set to oxygen, update to new theme which matches breeze theme

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QSettings>
#include <QDir>

Q_DECLARE_LOGGING_CATEGORY(GTKBREEZE)
Q_LOGGING_CATEGORY(GTKBREEZE, "gtkbreeze")

/*
 * returns a path to the installed gtk if it can be found
 * themeName: gtk theme
 * settingsFile: a file installed with the theme to set default options
 */
QString isGtkThemeInstalled(QString themeName, QString settingsFile)
{
    foreach (const QString& themesDir, QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "themes", QStandardPaths::LocateDirectory)) {
        if (QFile::exists(themesDir + QDir::separator() + themeName + QDir::separator() + settingsFile)) {
            return themesDir + "/" + themeName;
        }
    }
    return 0;
}

/*
 * Check if gtk theme is already set to oxygen, if it is then we want to upgrade to the breeze theme
 * gtkSettingsFile: filename to use
 * settingsKey: ini group to read from
 * returns: full path to settings file
 */
QString isGtkThemeSetToOxygen(QString gtkSettingsFile, QString settingsKey)
{
    QString gtkSettingsPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QDir::separator() + gtkSettingsFile;
    qCDebug(GTKBREEZE) << "looking for" << gtkSettingsPath;
    if (QFile::exists(gtkSettingsPath)) {
        qCDebug(GTKBREEZE) << "found settings file" << gtkSettingsPath;
        QSettings gtkrcSettings(gtkSettingsPath, QSettings::IniFormat);
        if (!settingsKey.isNull()) {
            gtkrcSettings.beginGroup(settingsKey);
        }
        if (gtkrcSettings.value("gtk-theme-name") != QStringLiteral("oxygen-gtk")) {
            qCDebug(GTKBREEZE) << "gtk settings file " + gtkSettingsFile + " already exists and is not using oxygen, will not change";
            return QString();
        }
    }
    return gtkSettingsPath;
}

/*
 * Set gtk2 theme if no theme is set or if oxygen is set and gtk theme is installed
 */
int setGtk2()
{
    const QString gtk2Theme = QStringLiteral("Orion"); // Orion looks kindae like breeze
    const QString gtk2ThemeSettings = QStringLiteral("gtk-2.0/gtkrc"); // file to check for
    const QString gtkThemeDirectory = isGtkThemeInstalled(gtk2Theme, gtk2ThemeSettings);

    if (gtkThemeDirectory == 0) {
        qCDebug(GTKBREEZE) << "not found, quitting";
        return 0;
    }
    qCDebug(GTKBREEZE) << "found gtktheme: " << gtkThemeDirectory;

    QString gtkrc2path = isGtkThemeSetToOxygen(".gtkrc-2.0", QString());
    if (gtkrc2path.isEmpty()) {
        qCDebug(GTKBREEZE) << "gtkrc2 already exists and is not using oxygen, quitting";
        return 0;
    }

    qCDebug(GTKBREEZE) << "no gtkrc2 file or oxygen being used, setting to new theme";
    QFile gtkrc2writer(gtkrc2path);
    bool opened = gtkrc2writer.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!opened) {
        qCWarning(GTKBREEZE) << "failed to open " << gtkrc2path;
        return 1;
    }
    QTextStream out(&gtkrc2writer);
    out << QStringLiteral("include \"") << gtkThemeDirectory << QStringLiteral("/gtk-2.0/gtkrc\"\n");
    out << QStringLiteral("style \"user-font\"\n");
    out << QStringLiteral("{\n");
    out << QStringLiteral("    font_name=\"Oxygen-Sans Sans-Book\"\n");
    out << QStringLiteral("}\n");
    out << QStringLiteral("widget_class \"*\" style \"user-font\"\n");
    out << QStringLiteral("gtk-font-name=\"Oxygen-Sans Sans-Book 10\"\n"); // matches plasma-workspace:startkde/startkde.cmake
    out << QStringLiteral("gtk-theme-name=\"Orion\"\n");
    out << QStringLiteral("gtk-icon-theme-name=\"oxygen\"\n"); // breeze icons don't seem to work with gtk
    out << QStringLiteral("gtk-fallback-icon-theme=\"gnome\"\n");
    out << QStringLiteral("gtk-toolbar-style=GTK_TOOLBAR_ICONS\n");
    out << QStringLiteral("gtk-menu-images=1\n");
    out << QStringLiteral("gtk-button-images=1\n");
    qCDebug(GTKBREEZE) << "gtk2rc written";
    return 0;
}

/*
 * Set gtk3 theme if no theme is set or if oxygen is set and gtk theme is installed
 */
int setGtk3()
{
    qCDebug(GTKBREEZE) << "setGtk3()";

    const QString gtk3Theme = QStringLiteral("Orion"); // Orion looks kindae like breeze
    const QString gtk3ThemeSettings = QStringLiteral("gtk-3.0/settings.ini"); // Orion looks kindae like breeze

    const QString gtkThemeDirectory = isGtkThemeInstalled(gtk3Theme, gtk3ThemeSettings);
    if (gtkThemeDirectory == 0) {
        qCDebug(GTKBREEZE) << "not found, quitting";
        return 0;
    }
    qCDebug(GTKBREEZE) << "found gtk3theme:" << gtkThemeDirectory;

    QString gtkrc3path = isGtkThemeSetToOxygen(".config/gtk-3.0/settings.ini", "Settings");
    if ( gtkrc3path.isEmpty() ) {
        qCDebug(GTKBREEZE) << "gtkrc3 already exists and is not using oxygen, quitting";
        return 0;
    }

    qCDebug(GTKBREEZE) << "no gtkrc3 file or oxygen being used, setting to new theme";
    QFile gtkrc3writer(gtkrc3path);
    bool opened = gtkrc3writer.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!opened) {
        qCWarning(GTKBREEZE) << "failed to open " << gtkrc3path;
        return 1;
    }
    QTextStream out(&gtkrc3writer);
    out << QStringLiteral("[Settings]\n");
    out << QStringLiteral("gtk-font-name=Oxygen-Sans 10\n"); // matches plasma-workspace:startkde/startkde.cmake
    out << QStringLiteral("gtk-theme-name=")+gtk3Theme+QStringLiteral("\n");
    out << QStringLiteral("gtk-icon-theme-name=oxygen\n"); // breeze icons don't seem to work with gtk
    out << QStringLiteral("gtk-fallback-icon-theme=gnome\n");
    out << QStringLiteral("gtk-toolbar-style=GTK_TOOLBAR_ICONS\n");
    out << QStringLiteral("gtk-menu-images=1\n");
    out << QStringLiteral("gtk-button-images=1\n");
    qCDebug(GTKBREEZE) << "gtk3rc written";
    return 0;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("gtkbreeze.debug = true"));
    qCDebug(GTKBREEZE) << "updateGtk2()";

    int result = 0;
    result = setGtk2();
    result = setGtk3();

    return result;
}
