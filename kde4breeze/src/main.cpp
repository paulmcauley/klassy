/***************************************************************************
 *   Copyright (C) 2014 by Marco Martin <mart@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStyleFactory>
#include <kdelibs4migration.h>

#include <KSharedConfig>
#include <KConfigGroup>
#include <kconfig.h>


void applyColorScheme(KConfig *other)
{
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/Breeze.colors");

    KSharedConfigPtr config = KSharedConfig::openConfig(src);

    foreach (const QString &grp, config->groupList()) {
        KConfigGroup cg(config, grp);
        KConfigGroup cg2(other, grp);
        cg.copyTo(&cg2);
    }
}

void cloneColorScheme()
{
    Kdelibs4Migration migration;
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/Breeze.colors");
    QString dest = migration.saveLocation("data", "color-schemes") + "Breeze.colors";

    QFile::remove(dest);
    QFile::copy(src, dest);
}

void updateKdeGlobals()
{
    Kdelibs4Migration migration;
    //Apply Breeze color scheme
    KConfig config(migration.saveLocation("config") + "kdeglobals");

    //use QtCurve only if installed
    const bool hasQtCurve = QStyleFactory::keys().contains("QtCurve");
    KConfigGroup group(&config, "General");
    group.writeEntry("ColorScheme", "Breeze");
    if (hasQtCurve) {
        group.writeEntry("widgetStyle", "qtcurve");
    }
    applyColorScheme(&config);
    group.sync();

    KConfigGroup iconGroup(&config, "Icons");
    iconGroup.writeEntry("Theme", "breeze");
    applyColorScheme(&config);
    iconGroup.sync();

    KSharedConfig::Ptr kf5Config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup kf5Group(kf5Config, "General");
    kf5Group.writeEntry("ColorScheme", "Breeze");
    if (hasQtCurve) {
        kf5Group.writeEntry("widgetStyle", "qtcurve");
    }
    applyColorScheme(kf5Group.config());
    kf5Group.sync();

    KConfigGroup kf52Group(kf5Config, "KDE");
    kf52Group.writeEntry("ColorScheme", "Breeze");
    if (hasQtCurve) {
        kf52Group.writeEntry("widgetStyle", "qtcurve");
    }
    applyColorScheme(kf52Group.config());
    kf52Group.sync();

    KConfigGroup kf5IconGroup(kf5Config, "Icons");
    kf5IconGroup.writeEntry("Theme", "breeze");
    kf5IconGroup.sync();
}

void applyQtCurveConfig()
{
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "QtCurve/Breeze.qtcurve");
    QString dest = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/qtcurve/stylerc";

    //create target directory otherwise copy fails
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
    dir.mkpath("qtcurve");

    QFile::remove(dest);
    QFile::copy(src, dest);
}


int main(int argc, char **argv)
{

    QCoreApplication app(argc, argv);


    cloneColorScheme();
    updateKdeGlobals();
    applyQtCurveConfig();

    return 0;
}
