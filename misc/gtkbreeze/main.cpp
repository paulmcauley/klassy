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
 * Set gtk2 theme to qtcurve if no theme is set or if oxygen is set and qtcurve is installed
 */
void updateGtk2()
{
    QLoggingCategory::setFilterRules(QStringLiteral("gtkbreeze.debug = true"));
    qCDebug(GTKBREEZE) << "updateGtk2()";

    // check if qtcurve gtk2 engine is installed by looking for /usr/share/themes/QtCurve/
    QFileInfoList availableThemes;
    bool foundQtCurve = false;
    foreach(const QString& themesDir, QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "themes", QStandardPaths::LocateDirectory)) {
        QDir root(themesDir);
        qCDebug(GTKBREEZE) << "found: " << root.dirName();
        availableThemes = root.entryInfoList(QDir::NoDotAndDotDot|QDir::AllDirs);
        foreach(const QFileInfo& themeDir, availableThemes) {
            qCDebug(GTKBREEZE) << "found: " << themeDir.fileName();
            if (themeDir.fileName() == "QtCurve") {
                foundQtCurve = true;
                break;
            }                
        }
    }
    if (!foundQtCurve) {
        qCDebug(GTKBREEZE) << "not found, quitting";
        return;
    }
    qCDebug(GTKBREEZE) << "found qtcurve";
    
    QString gtkrc2path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QString("/.gtkrc-2.0");
    qCDebug(GTKBREEZE) << "looking for" << gtkrc2path;
    if (QFile::exists(gtkrc2path)) {
        //check for oxygen
        qCDebug(GTKBREEZE) << "found ~/.gtkrc-2.0";
        QSettings gtkrc2(gtkrc2path, QSettings::IniFormat);
        if (gtkrc2.value("gtk-theme-name") != "oxygen-gtk") {
            qCDebug(GTKBREEZE) << "gtkrc2 already exist and is not using oxygen, quitting";
            return;            
        }
    }
    qCDebug(GTKBREEZE) << "no gtkrc2 file or oxygen being used, setting to qtcurve";
    /*
    Kdelibs4Migration migration;
    //Apply the color scheme
    KConfig config(migration.saveLocation("config") + "kdeglobals");

    KSharedConfig::Ptr kf5Config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup kf5Group(kf5Config, "General");
    KConfigGroup kf52Group(kf5Config, "KDE");

    const QString looknfeel = kf52Group.readEntry("LookAndFeelPackage", defaultLookAndFeelPackage);

    KSharedConfigPtr lnfConfig;
    KSharedConfigPtr defaultLnfConfig = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/look-and-feel/" + defaultLookAndFeelPackage + "/contents/defaults"));
    if (looknfeel != defaultLookAndFeelPackage) {
        lnfConfig = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/look-and-feel/" + looknfeel + "/contents/defaults"));
    }

    const QString widgetStyle = readConfigValue(lnfConfig, defaultLnfConfig, "KDE", "widgetStyle", "breeze").toString();
    const QString colorScheme = readConfigValue(lnfConfig, defaultLnfConfig, "General", "ColorScheme", "Breeze").toString();
    const QString icons = readConfigValue(lnfConfig, defaultLnfConfig, "Icons", "Theme", "breeze").toString();

    cloneColorScheme(colorScheme);

    //use only if the style from the look and feel package is installed
    const bool hasWidgetStyle = QStyleFactory::keys().contains(widgetStyle);
    KConfigGroup group(&config, "General");
    group.writeEntry("ColorScheme", colorScheme);
    group.sync();
    if (hasWidgetStyle) {
        group.writeEntry("widgetStyle", widgetStyle);
    }
    applyColorScheme(colorScheme, &config);
    group.sync();

    KConfigGroup iconGroup(&config, "Icons");
    iconGroup.writeEntry("Theme", icons);
    applyColorScheme(colorScheme, &config);
    iconGroup.sync();


    kf5Group.writeEntry("ColorScheme", colorScheme);
    kf5Group.sync();
    if (hasWidgetStyle) {
        kf5Group.writeEntry("widgetStyle", widgetStyle);
    }
    applyColorScheme(colorScheme, kf5Group.config());
    kf5Group.sync();


    kf52Group.writeEntry("ColorScheme", colorScheme);
    if (hasWidgetStyle) {
        kf52Group.writeEntry("widgetStyle", widgetStyle);
    }
    applyColorScheme(colorScheme, kf52Group.config());
    kf52Group.sync();

    KConfigGroup kf5IconGroup(kf5Config, "Icons");
    kf5IconGroup.writeEntry("Theme", icons);
    kf5IconGroup.sync();
    */
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    updateGtk2();

    return 0;
}
