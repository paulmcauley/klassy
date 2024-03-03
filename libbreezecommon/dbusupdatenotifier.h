/*
 * SPDX-FileCopyrightText: 2024 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "breeze.h"
#include "breezecommon_export.h"
#include <QDBusVariant>
#include <QString>

namespace Breeze
{

class BREEZECOMMON_EXPORT DBusUpdateNotifier : public QObject
{
    Q_OBJECT

public:
    DBusUpdateNotifier();

public Q_SLOTS:
    void onWindowDecorationSettingsUpdate();
    void onSystemSettingUpdate(QString, QString, QDBusVariant);

Q_SIGNALS:
    void decorationSettingsUpdate(QByteArray uuid);
    void systemColorSchemeUpdate(QByteArray uuid);
    void systemIconsUpdate();
};

extern DBusUpdateNotifier BREEZECOMMON_EXPORT g_dBusUpdateNotifier;

}
