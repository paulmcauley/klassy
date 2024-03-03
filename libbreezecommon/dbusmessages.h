/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QDBusConnection>
#include <QDBusMessage>

namespace Breeze
{

class DBusMessages
{
public:
    static void kwinReloadConfig()
    {
        // needed to tell Decoration::reconfigure to reload when running from external kcmshell
        QDBusMessage message(QDBusMessage::createSignal(QStringLiteral("/KWin"), QStringLiteral("org.kde.KWin"), QStringLiteral("reloadConfig")));
        QDBusConnection::sessionBus().send(message);
    }

    static void kstyleReloadDecorationConfig()
    {
        // needed for klassy application style to reload shadows
        QDBusMessage message(
            QDBusMessage::createSignal(QStringLiteral("/KlassyDecoration"), QStringLiteral("org.kde.Klassy.Style"), QStringLiteral("reparseConfiguration")));
        QDBusConnection::sessionBus().send(message);
    }

    static void kstyleReloadConfig()
    {
        QDBusMessage message(
            QDBusMessage::createSignal(QStringLiteral("/KlassyStyle"), QStringLiteral("org.kde.Klassy.Style"), QStringLiteral("reparseConfiguration")));
        QDBusConnection::sessionBus().send(message);
    }

    static void updateDecorationColorCache()
    {
        QDBusMessage message(QDBusMessage::createSignal(QStringLiteral("/KlassyDecoration"),
                                                        QStringLiteral("org.kde.Klassy.Style"),
                                                        QStringLiteral("updateDecorationColorCache")));
        QDBusConnection::sessionBus().send(message);
    }

    static void setGtkTheme(QString themeName)
    {
        QDBusMessage message(QDBusMessage::createMethodCall(QStringLiteral("org.kde.GtkConfig"),
                                                            QStringLiteral("/GtkConfig"),
                                                            QStringLiteral("org.kde.GtkConfig"),
                                                            QStringLiteral("setGtkTheme")));
        message.setArguments(QList{QVariant(themeName)});
        QDBusConnection::sessionBus().send(message);
    }
};

}
