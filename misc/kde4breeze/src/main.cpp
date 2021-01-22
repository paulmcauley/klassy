/*
 *   SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

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

static const QString defaultLookAndFeelPackage = "org.kde.breeze.desktop";

void applyColorScheme(const QString &colorScheme, KConfig *other)
{
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/" + colorScheme + ".colors");

    KSharedConfigPtr config = KSharedConfig::openConfig(src);

    foreach (const QString &grp, config->groupList()) {
        KConfigGroup cg(config, grp);
        KConfigGroup cg2(other, grp);
        cg.copyTo(&cg2, KConfigBase::Notify);
    }
}

void cloneColorScheme(const QString &colorScheme)
{
    Kdelibs4Migration migration;
    QString src = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/" + colorScheme + ".colors");
    QString dest = migration.saveLocation("data", "color-schemes") + colorScheme + ".colors";

    QDir dir;
    dir.mkpath(migration.saveLocation("data", "color-schemes"));
    QFile::remove(dest);
    QFile::copy(src, dest);
}

QVariant readConfigValue(KSharedConfigPtr lnfConfig, KSharedConfigPtr defaultLnfConfig,
                         const QString &group, const QString &key, const QVariant &defaultValue)
{
    QVariant value;

    if (lnfConfig) {
        KConfigGroup lnfCg(lnfConfig, "kdeglobals");
        lnfCg = KConfigGroup(&lnfCg, group);
        if (lnfCg.isValid()) {
            value = lnfCg.readEntry(key, defaultValue);

            if (!value.isNull()) {
                return value;
            }
        }
    }

    KConfigGroup lnfCg(defaultLnfConfig, "kdeglobals");
    lnfCg = KConfigGroup(defaultLnfConfig, group);
    if (lnfCg.isValid()) {
        return lnfCg.readEntry(key, defaultValue);
    }

    return defaultValue;
}

void updateKdeGlobals()
{
    Kdelibs4Migration migration;
    //Apply the color scheme
    KConfig config(migration.saveLocation("config") + "kdeglobals", KConfig::SimpleConfig);

    KSharedConfig::Ptr kf5Config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup kf5Group(kf5Config, "General");
    KConfigGroup kf52Group(kf5Config, "KDE");

    const QString looknfeel = kf52Group.readEntry("LookAndFeelPackage", defaultLookAndFeelPackage);

    KSharedConfigPtr lnfConfig;
    KSharedConfigPtr defaultLnfConfig = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/look-and-feel/" + defaultLookAndFeelPackage + "/contents/defaults"));
    if (looknfeel != defaultLookAndFeelPackage) {
        lnfConfig = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/look-and-feel/" + looknfeel + "/contents/defaults"));
    }

    const QString widgetStyle = readConfigValue(lnfConfig, defaultLnfConfig, "KDE", "widgetStyle", "Breeze").toString();
    const QString colorScheme = readConfigValue(lnfConfig, defaultLnfConfig, "General", "ColorScheme", "BreezeLight").toString();
    const QString icons = readConfigValue(lnfConfig, defaultLnfConfig, "Icons", "Theme", "breeze").toString();

    cloneColorScheme(colorScheme);

    //use only if the style from the look and feel package is installed
    const bool hasWidgetStyle = QStyleFactory::keys().contains(widgetStyle);
    KConfigGroup group(&config, "General");
    group.writeEntry("ColorScheme", colorScheme);

    qDebug() << "setting widget style:" << widgetStyle << hasWidgetStyle;
    if (hasWidgetStyle) {
        group.writeEntry("widgetStyle", widgetStyle);
        //for some reason this seems necessary
        group.sync();
    }
    applyColorScheme(colorScheme, &config);
    group.sync();

    KConfigGroup iconGroup(&config, "Icons");
    iconGroup.writeEntry("Theme", icons);
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
}

int main(int argc, char **argv)
{

    QCoreApplication app(argc, argv);

    updateKdeGlobals();

    return 0;
}
