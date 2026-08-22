/******************************************************************
 * Copyright 2016 Chinmoy Ranjan Pradhan <chinmoyrp65@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************/

#pragma once

#include "breezedecoration.h"

// Qt
#include <QAction>
#include <QDBusServiceWatcher>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QtTypes>

namespace Breeze
{

class KDBusMenuImporter;

class AppMenuModel : public QObject
{
    Q_OBJECT

public:
    explicit AppMenuModel(QObject *parent, Decoration *decoration);
    ~AppMenuModel() override;

public:
    void updateApplicationMenu(const QString &serviceName, const QString &menuObjectPath);

    QMenu *menu() const;

private:
    void update();

signals:
    void menuAvailableChanged();
    void modelNeedsUpdate();
    void modelReset();
    void menuReadyForSearch();
    void subMenuReady(QMenu *menu);

public:
    void loadSubMenu(QMenu *menu);
    void stopCaching();
    void startDeepCaching();

private:
    void onMenuUpdated(QMenu *menu);
    void onActionChanged();
    void processNext();

private:
    void registerSubMenus(QMenu *menu);
    void resumeDeepCacheIfIdle(QMenu *menu);
    bool menuAvailable() const;
    void setMenuAvailable(bool set);

    QTimer *m_staggerTimer;
    QList<QPointer<QMenu>> m_menusToDeepCache;
    qsizetype m_nextMenuToProcess = 0;
    QSet<QMenu *> m_seenMenus;
    bool m_menuAvailable = false;
    bool m_deepCacheRequested = false;
    bool m_deepCacheStarted = false;
    QSet<QMenu *> m_pendingDeepCacheUpdates;
    bool m_updatePending = false;

    QPointer<QMenu> m_menu;

    QDBusServiceWatcher *m_serviceWatcher;
    QString m_serviceName;
    QString m_menuObjectPath;

    QPointer<KDBusMenuImporter> m_importer;
    Decoration *m_decoration;
};

} // namespace Breeze
