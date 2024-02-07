/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "dbuslistener.h"
#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

DBusUpdateNotifier g_dBusUpdateNotifier;

DBusUpdateNotifier::DBusUpdateNotifier()
{
    QDBusConnection dBusConnection = QDBusConnection::sessionBus();

    dBusConnection.connect(
        QString(),
        QStringLiteral("/org/freedesktop/portal/desktop"),
        QStringLiteral("org.freedesktop.portal.Settings"), // there is also an org.freedesktop.impl.portal.Settings interface with the same data
        QStringLiteral("SettingChanged"),
        this,
        SLOT(onSystemSettingUpdate(QString, QString, QDBusVariant)));

    dBusConnection.connect(QString(),
                           QStringLiteral("/KlassyDecoration"),
                           QStringLiteral("org.kde.Klassy.Style"),
                           QStringLiteral("updateDecorationColorCache"),
                           this,
                           SLOT(onWindowDecorationSettingsUpdate()));
}

void DBusUpdateNotifier::onWindowDecorationSettingsUpdate()
{
    Q_EMIT decorationSettingsUpdate(QUuid::createUuid().toByteArray());
}

void DBusUpdateNotifier::onSystemSettingUpdate(QString first, QString second, QDBusVariant third)
{
    Q_UNUSED(third);
    if (first == QStringLiteral("org.freedesktop.appearance") && second == QStringLiteral("color-scheme")) { // third is an int
        Q_EMIT systemColorSchemeUpdate(QUuid::createUuid().toByteArray());
    } else if (first == QStringLiteral("org.gnome.desktop.interface") && second == QStringLiteral("icon-theme")) { // third is a string with the icon theme name
        Q_EMIT systemIconsUpdate();
    }
}

}
