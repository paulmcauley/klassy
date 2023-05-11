
//////////////////////////////////////////////////////////////////////////////
// breezedetectwidget.cpp
// Note: this class is a stripped down version of
// /kdebase/workspace/kwin/kcmkwin/kwinrules/detectwidget.cpp
// SPDX-FileCopyrightText: 2004 Lubos Lunak <l.lunak@kde.org>
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "breezedetectwidget.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace Breeze
{

//_________________________________________________________
DetectDialog::DetectDialog(QObject *parent)
    : QObject(parent)
{
}

//_________________________________________________________
void DetectDialog::detect()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("/KWin"),
                                                          QStringLiteral("org.kde.KWin"),
                                                          QStringLiteral("queryWindowInfo"));

    QDBusPendingReply<QVariantMap> asyncReply = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncReply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QVariantMap> reply = *self;
        self->deleteLater();
        if (!reply.isValid()) {
            Q_EMIT detectionDone(false);
            return;
        }
        m_properties = reply.value();
        Q_EMIT detectionDone(true);
    });
}

//_________________________________________________________
QVariantMap DetectDialog::properties() const
{
    return m_properties;
}
}
