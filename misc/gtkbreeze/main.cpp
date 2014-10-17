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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStyleFactory>
#include <QLoggingCategory>
#include <QSettings>

#include <KSharedConfig>
#include <KConfigGroup>
#include <kconfig.h>

Q_DECLARE_LOGGING_CATEGORY(GTKBREEZE)
Q_LOGGING_CATEGORY(GTKBREEZE, "gtkbreeze")

/*
 * Set gtk2 theme if no theme is set or if oxygen is set and gtk theme is installed
 */
void setGtk2()
{
    QString gtk2Theme = "Orion"; // Orion looks kindae like breeze

    // check if gtk engine is installed by looking in /usr/share/themes/
    QFileInfoList availableThemes;
    QString gtkThemeDirectory;
    foreach(const QString& themesDir, QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "themes", QStandardPaths::LocateDirectory)) {
        if (QFile::exists(themesDir+ "/" + gtk2Theme + "/gtk-2.0/gtkrc")) {
            gtkThemeDirectory = themesDir + "/" + gtk2Theme;
            qCDebug(GTKBREEZE) << "setting gtkThemeDirectory: " << gtkThemeDirectory;
            break;
        }
    }
    if (gtkThemeDirectory.isEmpty()) {
        qCDebug(GTKBREEZE) << "not found, quitting";
        return;
    }
    qCDebug(GTKBREEZE) << "found gtktheme";

    QString gtkrc2path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QString("/.gtkrc-2.0");
    qCDebug(GTKBREEZE) << "looking for" << gtkrc2path;
    if (QFile::exists(gtkrc2path)) {
        //check for oxygen
        qCDebug(GTKBREEZE) << "found ~/.gtkrc-2.0";
        QSettings gtkrc2settings(gtkrc2path, QSettings::IniFormat);
        if (gtkrc2settings.value("gtk-theme-name") != "oxygen-gtk") {
            qCDebug(GTKBREEZE) << "gtkrc2 already exist and is not using oxygen, quitting";
            return;
        }
    }
    qCDebug(GTKBREEZE) << "no gtkrc2 file or oxygen being used, setting to new theme";
    QFile gtkrc2writer(gtkrc2path);
    gtkrc2writer.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&gtkrc2writer);
    out << "include \"" << gtkThemeDirectory << "/gtk-2.0/gtkrc\"\n";
    out << "style \"user-font\"\n";
    out << "{\n";
    out << "    font_name=\"Oxygen-Sans Sans-Book\"\n";
    out << "}\n";
    out << "widget_class \"*\" style \"user-font\"\n";
    out << "gtk-font-name=\"Oxygen-Sans Sans-Book 10\"\n";  // matches plasma-workspace:startkde/startkde.cmake
    out << "gtk-theme-name=\"Orion\"\n";
    out << "gtk-icon-theme-name=\"breeze\"\n";
    out << "gtk-fallback-icon-theme=\"oxygen\"\n";
    out << "gtk-toolbar-style=GTK_TOOLBAR_ICONS\n";
    out << "gtk-menu-images=1\n";
    out << "gtk-button-images=1\n";

    gtkrc2writer.close();
    qCDebug(GTKBREEZE) << "gtk2rc written";
}

/*
 * Set gtk3 theme if no theme is set or if oxygen is set and gtk theme is installed
 */
void setGtk3()
{
    qCDebug(GTKBREEZE) << "setGtk3()";

    QString gtk3Theme = "Orion"; // Orion looks kindae like breeze
/*
    // check if gtk engine is installed by looking in /usr/share/themes/
    QFileInfoList availableThemes;
    QString gtkThemeDirectory;
    foreach(const QString& themesDir, QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "themes", QStandardPaths::LocateDirectory)) {
        QDir root(themesDir);
        qCDebug(GTKBREEZE) << "found: " << root.dirName();
        availableThemes = root.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);
        foreach(const QFileInfo& themeDir, availableThemes) {
            qCDebug(GTKBREEZE) << "found: " << themeDir.fileName();
            if (themeDir.fileName() == gtk2Theme) {
                gtkThemeDirectory = themeDir.filePath();
                qCDebug(GTKBREEZE) << "setting gtkThemeDirectory: " << gtkThemeDirectory;
                break;
            }
        }
    }
    if (gtkThemeDirectory.isEmpty()) {
        qCDebug(GTKBREEZE) << "not found, quitting";
        return;
    }
    qCDebug(GTKBREEZE) << "found gtktheme";
    
    QString gtkrc2path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QString("/.gtkrc-2.0");
    qCDebug(GTKBREEZE) << "looking for" << gtkrc2path;
    if (QFile::exists(gtkrc2path)) {
        //check for oxygen
        qCDebug(GTKBREEZE) << "found ~/.gtkrc-2.0";
        QSettings gtkrc2settings(gtkrc2path, QSettings::IniFormat);
        if (gtkrc2settings.value("gtk-theme-name") != "oxygen-gtk") {
            qCDebug(GTKBREEZE) << "gtkrc2 already exist and is not using oxygen, quitting";
            return;            
        }
    }
    qCDebug(GTKBREEZE) << "no gtkrc2 file or oxygen being used, setting to qtcurve";
    QFile gtkrc2writer(gtkrc2path);
    gtkrc2writer.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&gtkrc2writer);
    out << "include \"" << gtkThemeDirectory << "/gtk-2.0/gtkrc\"\n";
    out << "style \"user-font\"\n";
    out << "{\n";
    out << "    font_name=\"Oxygen-Sans Sans-Book\"\n";
    out << "}\n";
    out << "widget_class \"*\" style \"user-font\"\n";
    out << "gtk-font-name=\"Oxygen-Sans Sans-Book 10\"\n";  // matches plasma-workspace:startkde/startkde.cmake
    out << "gtk-theme-name=\"Orion\"\n";
    out << "gtk-icon-theme-name=\"breeze\"\n";
    out << "gtk-fallback-icon-theme=\"oxygen\"\n";
    out << "gtk-toolbar-style=GTK_TOOLBAR_ICONS\n";
    out << "gtk-menu-images=1\n";
    out << "gtk-button-images=1\n";

    gtkrc2writer.close();
    qCDebug(GTKBREEZE) << "gtk2rc written";
    */
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("gtkbreeze.debug = true"));
    qCDebug(GTKBREEZE) << "updateGtk2()";

    setGtk2();
    setGtk3();

    return 0;
}
