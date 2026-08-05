/******************************************************************
 * Copyright 2025 Guido Iodice <guido.iodice@gmail.com>
 * Copyright 2016 Kai Uwe Broulik <kde@privat.broulik.de>
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

// Based on:
// https://invent.kde.org/plasma/plasma-workspace/-/blob/master/applets/appmenu/plugin/appmenumodel.cpp
// https://github.com/psifidotos/applet-window-appmenu/blob/master/plugin/appmenumodel.cpp

// own
#include "appmenu/model.h"
#include "appmenu/navigablemenu.h"

// Qt
#include <QAction>
#include <QDeadlineTimer>
#include <QMenu>

#include <QDBusConnection>
#include <QDBusServiceWatcher>

// libdbusmenuqt
#include <dbusmenuimporter.h>

// std
#include <utility>

namespace Breeze
{

class KDBusMenuImporter : public DBusMenuImporter
{
public:
    KDBusMenuImporter(QStyle *menuStyle, const QString &service, const QString &path, QObject *parent)
        : DBusMenuImporter(service, path, parent)
        , m_menuStyle(menuStyle)
    {
    }

protected:
    QIcon iconForName(const QString &name) override
    {
        return QIcon::fromTheme(name);
    }

    QMenu *createMenu(QWidget *parent) override
    {
        return new NavigableMenu(parent, m_menuStyle);
    }

private:
    QStyle *m_menuStyle;
};

AppMenuModel::AppMenuModel(QObject *parent, QStyle *menuStyle)
    : QObject(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(this))
    , m_menuStyle(menuStyle)
{
    m_serviceWatcher->setConnection(QDBusConnection::sessionBus());
    // If our current DBus connection gets lost, close the menu
    // we'll select the new menu when the focus changes
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString &serviceName) {
        if (serviceName == m_serviceName) {
            setMenuAvailable(false);
            stopCaching();
            m_serviceName.clear();
            m_menuObjectPath.clear();
            if (m_importer) {
                m_importer->deleteLater();
                m_importer = nullptr;
            }
            Q_EMIT modelNeedsUpdate();
        }
    });

    connect(this, &AppMenuModel::modelNeedsUpdate, this, [this] {
        if (!m_updatePending) {
            m_updatePending = true;
            QMetaObject::invokeMethod(this, &AppMenuModel::update, Qt::QueuedConnection);
        }
    });

    m_staggerTimer = new QTimer(this);
    m_staggerTimer->setSingleShot(true);
    m_staggerTimer->setInterval(8);
    connect(m_staggerTimer, &QTimer::timeout, this, &AppMenuModel::processNext);
}

AppMenuModel::~AppMenuModel()
{
    stopCaching();
}

bool AppMenuModel::menuAvailable() const
{
    return m_menuAvailable;
}

void AppMenuModel::setMenuAvailable(bool set)
{
    if (m_menuAvailable != set) {
        m_menuAvailable = set;
        Q_EMIT menuAvailableChanged();
    }
}

QMenu *AppMenuModel::menu() const
{
    return m_menu;
}

void AppMenuModel::update()
{
    Q_EMIT modelReset();
    m_updatePending = false;
}

void AppMenuModel::updateApplicationMenu(const QString &serviceName, const QString &menuObjectPath)
{
    // A menu change means any in-progress caching is now invalid.
    stopCaching();

    if (m_serviceName == serviceName && m_menuObjectPath == menuObjectPath) {
        if (m_importer) {
            QMetaObject::invokeMethod(m_importer.data(), qOverload<>(&DBusMenuImporter::updateMenu), Qt::QueuedConnection);
            return;
        }
    }

    m_serviceName = serviceName;
    m_serviceWatcher->setWatchedServices({m_serviceName});

    m_menuObjectPath = menuObjectPath;
    m_menu = nullptr;

    if (m_importer) {
        m_importer->disconnect(this);
        m_importer->deleteLater();
    }

    m_importer = new KDBusMenuImporter(m_menuStyle, serviceName, menuObjectPath, this);
    QMetaObject::invokeMethod(m_importer.data(), qOverload<>(&DBusMenuImporter::updateMenu), Qt::QueuedConnection);

    connect(m_importer.data(), &DBusMenuImporter::menuUpdated, this, &AppMenuModel::onMenuUpdated);

    Q_EMIT modelNeedsUpdate();
}

void AppMenuModel::onActionChanged()
{
    // This is called when a top-level action changes (e.g., text, icon, enabled state).
    // We emit modelNeedsUpdate to eventually trigger a modelReset, but we could
    // add more sophisticated filtering here if needed.
    Q_EMIT modelNeedsUpdate();
}

void AppMenuModel::onMenuUpdated(QMenu *menu)
{
    // This slot is called by the DBusMenuImporter whenever a menu's contents are ready.
    if (m_menu.isNull() || menu == m_menu.data()) { // First time update, or a top-level menu update.
        m_menu = menu;
        if (m_menu.isNull()) {
            return;
        }

        // Connect signals for top-level actions to update the model when they change.
        for (QAction *a : m_menu->actions()) {
            connect(a, &QAction::destroyed, this, &AppMenuModel::modelNeedsUpdate, Qt::UniqueConnection);
            connect(a, &QAction::changed, this, &AppMenuModel::onActionChanged, Qt::UniqueConnection);
        }

        setMenuAvailable(true);
        Q_EMIT modelNeedsUpdate();

        // Pre-fetching and deep caching are now handled on-demand.
        if (m_deepCacheRequested) {
            resumeDeepCacheIfIdle(m_menu.data());
        }
    } else { // This is an update for a submenu that was previously requested.
        Q_EMIT subMenuReady(menu);
        if (m_deepCacheRequested) {
            resumeDeepCacheIfIdle(menu);
        }

        // Track the specific submenus being deep cached. When all pending
        // updates are finished, the entire menu tree has been fetched.
        if (m_pendingDeepCacheUpdates.remove(menu)) {
            if (m_pendingDeepCacheUpdates.isEmpty() && m_nextMenuToProcess >= m_menusToDeepCache.size()) {
                processNext();
            }
        }
    }
}

void AppMenuModel::loadSubMenu(QMenu *menu)
{
    if (m_importer && menu) {
        m_importer->updateMenu(menu);
    }
}

void AppMenuModel::stopCaching()
{
    for (QMenu *subMenu : std::as_const(m_seenMenus)) {
        disconnect(subMenu, nullptr, this, nullptr);
    }
    m_seenMenus.clear();

    m_menusToDeepCache.clear();
    m_nextMenuToProcess = 0;
    m_staggerTimer->stop();
    m_deepCacheRequested = false;
    m_deepCacheStarted = false;
    m_pendingDeepCacheUpdates.clear();
}

void AppMenuModel::startDeepCaching()
{
    if (m_deepCacheStarted) {
        return;
    }

    m_deepCacheRequested = true;

    if (!m_menu) {
        return;
    }

    m_deepCacheStarted = true;
    m_menusToDeepCache.clear();
    m_nextMenuToProcess = 0;
    m_seenMenus.clear();

    // Populate the queue with the first level of submenus.
    // The recursive loading will happen as each menu is processed.
    registerSubMenus(m_menu.data());

    // Start processing the queue.
    processNext();
}

void AppMenuModel::registerSubMenus(QMenu *menu)
{
    if (!menu) {
        return;
    }
    for (QAction *a : menu->actions()) {
        if (auto subMenu = a->menu()) {
            if (!m_seenMenus.contains(subMenu)) {
                m_seenMenus.insert(subMenu);
                connect(subMenu, &QObject::destroyed, this, [this, subMenu]() {
                    m_seenMenus.remove(subMenu);
                    if (m_pendingDeepCacheUpdates.remove(subMenu)) {
                        if (m_pendingDeepCacheUpdates.isEmpty() && m_nextMenuToProcess >= m_menusToDeepCache.size()) {
                            QMetaObject::invokeMethod(this, &AppMenuModel::processNext, Qt::QueuedConnection);
                        }
                    }
                });
                m_menusToDeepCache.append(QPointer(subMenu));
            }
        }
    }
}

void AppMenuModel::resumeDeepCacheIfIdle(QMenu *menu)
{
    if (!menu || !m_deepCacheRequested) {
        return;
    }

    const bool wasQueueFinished = (m_nextMenuToProcess >= m_menusToDeepCache.size());

    registerSubMenus(menu);

    if (wasQueueFinished) {
        m_deepCacheStarted = true;
        processNext();
    }
}

void AppMenuModel::processNext()
{
    if (!m_deepCacheRequested) {
        return;
    }

    QDeadlineTimer deadline(std::chrono::milliseconds(8));

    while (!m_menusToDeepCache.isEmpty()) {
        if (m_nextMenuToProcess >= m_menusToDeepCache.size()) {
            if (!m_pendingDeepCacheUpdates.isEmpty()) {
                return; // Wait for pending updates to finish and potentially add more items
            }
            for (QMenu *subMenu : std::as_const(m_seenMenus)) {
                disconnect(subMenu, nullptr, this, nullptr);
            }
            m_menusToDeepCache.clear();
            m_nextMenuToProcess = 0;
            m_deepCacheRequested = false;
            m_deepCacheStarted = false;
            m_seenMenus.clear();
            Q_EMIT menuReadyForSearch();
            return;
        }

        QPointer<QMenu> menuToProcessPtr = m_menusToDeepCache.at(m_nextMenuToProcess++);
        QMenu *menuToProcess = menuToProcessPtr.data();

        if (menuToProcess) {
            if (!menuToProcess->actions().isEmpty()) {
                registerSubMenus(menuToProcess);
                if (deadline.hasExpired()) {
                    m_staggerTimer->start();
                    return;
                }
                continue; // Process next item immediately
            }

            m_pendingDeepCacheUpdates.insert(menuToProcess);
            if (m_importer) {
                m_importer->updateMenu(menuToProcess);
            }
            m_staggerTimer->start();
            return; // Wait for async update
        }
    }

    m_deepCacheRequested = false;
    m_deepCacheStarted = false;
    m_nextMenuToProcess = 0;
    for (QMenu *subMenu : std::as_const(m_seenMenus)) {
        disconnect(subMenu, nullptr, this, nullptr);
    }
    m_seenMenus.clear();
    if (m_pendingDeepCacheUpdates.isEmpty()) {
        Q_EMIT menuReadyForSearch();
    }
}

} // namespace Breeze
