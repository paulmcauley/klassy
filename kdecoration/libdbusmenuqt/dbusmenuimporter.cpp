/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenuimporter.h"

#include "debug.h"

// STL
#include <qbytearrayview.h>
#include <qlatin1stringview.h>
#include <utility>

// Qt
#include <QActionGroup>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDebug>
#include <QFont>
#include <QMenu>
#include <QPointer>
#include <QSet>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>

// Local
#include "dbusmenushortcut_p.h"
#include "dbusmenutypes_p.h"
#include "utils_p.h"

// Generated
#include "dbusmenu_interface.h"

// #define BENCHMARK
#ifdef BENCHMARK
static QTime sChrono;
#endif

#define DMRETURN_IF_FAIL(cond)                                                                                                                                 \
    if (!(cond)) {                                                                                                                                             \
        qCWarning(DBUSMENUQT) << "Condition failed: " #cond;                                                                                                   \
        return;                                                                                                                                                \
    }

static constexpr auto DBUSMENU_PROPERTY_ID = "_dbusmenu_id";
static constexpr auto DBUSMENU_PROPERTY_ICON_NAME = "_dbusmenu_icon_name";
static constexpr auto DBUSMENU_PROPERTY_ICON_DATA_HASH = "_dbusmenu_icon_data_hash";

static QAction *createKdeTitle(QAction *action, QWidget *parent)
{
    QToolButton *titleWidget = new QToolButton(nullptr);
    QFont font = titleWidget->font();
    font.setBold(true);
    titleWidget->setFont(font);
    titleWidget->setIcon(action->icon());
    titleWidget->setText(action->text());
    titleWidget->setDown(true);
    titleWidget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QWidgetAction *titleAction = new QWidgetAction(parent);
    titleAction->setDefaultWidget(titleWidget);
    return titleAction;
}

class DBusMenuImporterPrivate
{
public:
    DBusMenuImporter *q;

    DBusMenuInterface *m_interface = nullptr;
    QMenu *m_menu = nullptr;
    using ActionForId = QHash<int, QAction *>;
    ActionForId m_actionForId;
    QTimer m_pendingLayoutUpdateTimer;

    QSet<int> m_idsRefreshedByAboutToShow;
    QSet<int> m_pendingLayoutUpdates;

    QDBusPendingCallWatcher *refresh(int id)
    {
        const auto call = m_interface->GetLayout(id, 1, QStringList());
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, q);
        watcher->setProperty(DBUSMENU_PROPERTY_ID, id);
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q, &DBusMenuImporter::slotGetLayoutFinished);

        return watcher;
    }

    QMenu *createMenu(QWidget *parent)
    {
        QMenu *menu = q->createMenu(parent);
        QObject::connect(menu, &QMenu::hovered, q, &DBusMenuImporter::slotActionHovered);
        return menu;
    }

    /**
     * Init all the immutable action properties here
     * TODO: Document immutable properties?
     *
     * Note: we remove properties we handle from the map (using QMap::take()
     * instead of QMap::value()) to avoid warnings about these properties in
     * updateAction()
     */
    QAction *createAction(int id, const QVariantMap &_map, QWidget *parent)
    {
        QVariantMap map = _map;
        QAction *action = new QAction(parent);
        action->setProperty(DBUSMENU_PROPERTY_ID, id);

        const QString type = map.take(QStringLiteral("type")).toString();
        if (type == QLatin1StringView("separator")) {
            action->setSeparator(true);
        }

        const QString childrenDisplay = map.take(QStringLiteral("children-display")).toString();
        if (childrenDisplay == QLatin1StringView("submenu")) {
            QMenu *menu = createMenu(parent);
            menu->menuAction()->setProperty(DBUSMENU_PROPERTY_ID, id);
            action->setMenu(menu);
        } else if (!type.contains(QLatin1StringView("separator"))) {
            qCDebug(DBUSMENUQT) << "Action" << id << "has no submenu (children-display:" << childrenDisplay << ")";
        }

        const QString toggleType = map.take(QStringLiteral("toggle-type")).toString();
        if (!toggleType.isEmpty()) {
            action->setCheckable(true);
            if (toggleType == QLatin1StringView("radio")) {
                QActionGroup *group = new QActionGroup(action);
                group->addAction(action);
            }
        }

        const bool isKdeTitle = map.take(QStringLiteral("x-kde-title")).toBool();
        updateAction(action, map);

        if (isKdeTitle) {
            action = createKdeTitle(action, parent);
        }

        return action;
    }

    /**
     * Update mutable properties of an action.
     *
     * @param action the action to update
     * @param map holds the property values
     */
    void updateAction(QAction *action, const QVariantMap &map)
    {
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            const QString &key = it.key();
            if (key == QLatin1StringView("toggle-type") || key == QLatin1StringView("children-display")) {
                continue;
            }
            updateActionProperty(action, key, it.value());
        }
    }

    void updateActionProperty(QAction *action, const QString &key, const QVariant &value)
    {
        if (key == QLatin1StringView("type")) {
            updateActionType(action, value);
        } else if (key == QLatin1StringView("accessible-desc")) {
            updateActionAccessibilityDescription(action, value);
        } else if (key == QLatin1StringView("label")) {
            updateActionLabel(action, value);
        } else if (key == QLatin1StringView("enabled")) {
            updateActionEnabled(action, value);
        } else if (key == QLatin1StringView("toggle-state")) {
            updateActionChecked(action, value);
        } else if (key == QLatin1StringView("icon-name")) {
            updateActionIconByName(action, value);
        } else if (key == QLatin1StringView("icon-data")) {
            updateActionIconByData(action, value);
        } else if (key == QLatin1StringView("visible")) {
            updateActionVisible(action, value);
        } else if (key == QLatin1StringView("shortcut")) {
            updateActionShortcut(action, value);
        } else {
            qCWarning(DBUSMENUQT) << "Unhandled property update" << key;
        }
    }

    void updateActionType(QAction *action, const QVariant &value)
    {
        action->setSeparator(value == QLatin1StringView("separator"));
    }

    void updateActionAccessibilityDescription(QAction *action, const QVariant &value)
    {
        action->setIconText(value.toString());
    }

    void updateActionLabel(QAction *action, const QVariant &value)
    {
        QString text = swapMnemonicChar(value.toString(), '_', '&');
        if (action->menu()) {
            text += QLatin1StringView("  ");
        }
        action->setText(text);
    }

    void updateActionEnabled(QAction *action, const QVariant &value)
    {
        action->setEnabled(value.isValid() ? value.toBool() : true);
    }

    void updateActionChecked(QAction *action, const QVariant &value)
    {
        if (action->isCheckable() && value.isValid()) {
            action->setChecked(value.toInt() == 1);
        }
    }

    void updateActionIconByName(QAction *action, const QVariant &value)
    {
        const QString iconName = value.toString();
        const QString previous = action->property(DBUSMENU_PROPERTY_ICON_NAME).toString();
        if (previous == iconName) {
            return;
        }
        action->setProperty(DBUSMENU_PROPERTY_ICON_NAME, iconName);
        if (iconName.isEmpty()) {
            action->setIcon(QIcon());
            return;
        }
        action->setIcon(q->iconForName(iconName));
    }

    void updateActionIconByData(QAction *action, const QVariant &value)
    {
        const QByteArray data = value.toByteArray();
        uint dataHash = qHash(data);
        uint previousDataHash = action->property(DBUSMENU_PROPERTY_ICON_DATA_HASH).toUInt();
        if (previousDataHash == dataHash) {
            return;
        }
        action->setProperty(DBUSMENU_PROPERTY_ICON_DATA_HASH, dataHash);
        QPixmap pix;
        if (!pix.loadFromData(data)) {
            action->setIcon(QIcon());
            return;
        }
        action->setIcon(QIcon(pix));
    }

    void updateActionVisible(QAction *action, const QVariant &value)
    {
        action->setVisible(value.isValid() ? value.toBool() : true);
    }

    void updateActionShortcut(QAction *action, const QVariant &value)
    {
        QDBusArgument arg = value.value<QDBusArgument>();
        DBusMenuShortcut dmShortcut;
        arg >> dmShortcut;
        QKeySequence keySequence = dmShortcut.toKeySequence();
        action->setShortcut(keySequence);
    }

    QMenu *menuForId(int id) const
    {
        if (id == 0) {
            return q->menu();
        }
        QAction *action = m_actionForId.value(id);
        if (!action) {
            return nullptr;
        }
        return action->menu();
    }

    void slotItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList);

    void sendEvent(int id, const QString &eventId)
    {
        m_interface->Event(id, eventId, QDBusVariant(QString()), 0u);
    }
};

DBusMenuImporter::DBusMenuImporter(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<DBusMenuImporterPrivate>())
{
    DBusMenuTypes_register();

    d->q = this;
    d->m_interface = new DBusMenuInterface(service, path, QDBusConnection::sessionBus(), this);

    d->m_pendingLayoutUpdateTimer.setSingleShot(true);
    connect(&d->m_pendingLayoutUpdateTimer, &QTimer::timeout, this, &DBusMenuImporter::processPendingLayoutUpdates);

    connect(d->m_interface, &DBusMenuInterface::LayoutUpdated, this, &DBusMenuImporter::slotLayoutUpdated);
    connect(d->m_interface, &DBusMenuInterface::ItemActivationRequested, this, &DBusMenuImporter::slotItemActivationRequested);
    connect(d->m_interface,
            &DBusMenuInterface::ItemsPropertiesUpdated,
            this,
            [this](const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList) {
                d->slotItemsPropertiesUpdated(updatedList, removedList);
            });

    d->refresh(0);
}

DBusMenuImporter::~DBusMenuImporter()
{
    // Do not use "delete d->m_menu": even if we are being deleted we should
    // leave enough time for the menu to finish what it was doing, for example
    // if it was being displayed.
    if (d->m_menu) {
        // If StatusNotifierItemSource is gone before PlasmaDBusMenuImporter::menuUpdated, there is no menu
        d->m_menu->deleteLater();
    }
}

void DBusMenuImporter::slotLayoutUpdated(uint revision, int parentId)
{
    Q_UNUSED(revision)
    d->m_idsRefreshedByAboutToShow.remove(parentId);
    d->m_pendingLayoutUpdates << parentId;
    if (!d->m_pendingLayoutUpdateTimer.isActive()) {
        d->m_pendingLayoutUpdateTimer.start();
    }
}

void DBusMenuImporter::processPendingLayoutUpdates()
{
    QSet<int> ids;
    ids.swap(d->m_pendingLayoutUpdates);
    for (int id : std::as_const(ids)) {
        d->refresh(id);
    }
}

QMenu *DBusMenuImporter::menu() const
{
    if (!d->m_menu) {
        d->m_menu = d->createMenu(nullptr);
    }
    return d->m_menu;
}

void DBusMenuImporterPrivate::slotItemsPropertiesUpdated(const DBusMenuItemList &updatedList, const DBusMenuItemKeysList &removedList)
{
    bool needLayoutUpdate = false;
    for (const DBusMenuItem &item : updatedList) {
        QAction *action = m_actionForId.value(item.id);
        if (!action) {
            // We don't know this action. It probably is in a menu we haven't fetched yet.
            // This can happen if a new item is added but LayoutUpdated hasn't been received yet.
            needLayoutUpdate = true;
            continue;
        }

        auto it = item.properties.constBegin();
        const auto end = item.properties.constEnd();
        for (; it != end; ++it) {
            updateActionProperty(action, it.key(), it.value());
        }
    }

    for (const DBusMenuItemKeys &item : removedList) {
        QAction *action = m_actionForId.value(item.id);
        if (!action) {
            // We don't know this action. It probably is in a menu we haven't fetched yet.
            needLayoutUpdate = true;
            continue;
        }

        for (const QString &key : item.properties) {
            updateActionProperty(action, key, QVariant());
        }
    }

    if (needLayoutUpdate) {
        q->slotLayoutUpdated(0, 0);
    }
}

QAction *DBusMenuImporter::actionForId(int id) const
{
    return d->m_actionForId.value(id);
}

void DBusMenuImporter::slotItemActivationRequested(int id, uint /*timestamp*/)
{
    QAction *action = d->m_actionForId.value(id);
    DMRETURN_IF_FAIL(action);
    actionActivationRequested(action);
}

void DBusMenuImporter::slotGetLayoutFinished(QDBusPendingCallWatcher *watcher)
{
    const int parentId = watcher->property(DBUSMENU_PROPERTY_ID).toInt();
    watcher->deleteLater();

    d->m_idsRefreshedByAboutToShow.remove(parentId);

    QMenu *menu = d->menuForId(parentId);

    const QDBusPendingReply<uint, DBusMenuLayoutItem> reply = *watcher;
    if (!reply.isValid()) {
        qCWarning(DBUSMENUQT) << reply.error().message();
        if (menu) {
            Q_EMIT menuUpdated(menu);
        }
        return;
    }

#ifdef BENCHMARK
    qCDebug(DBUSMENUQT) << "- items received:" << sChrono.elapsed() << "ms";
#endif
    const DBusMenuLayoutItem rootItem = reply.argumentAt<1>();

    if (!menu) {
        qCDebug(DBUSMENUQT) << "No menu for id" << parentId;
        return;
    }

    menu->setUpdatesEnabled(false);

    const auto actions = menu->actions();
    QSet<int> newIds;
    newIds.reserve(rootItem.children.count());
    for (const auto &child : std::as_const(rootItem.children)) {
        newIds.insert(child.id);
    }

    // 1. Synchronize existing actions and add new ones
    auto currentActions = menu->actions();
    const int childCount = rootItem.children.count();
    for (int i = 0; i < childCount; ++i) {
        const DBusMenuLayoutItem &dbusMenuItem = rootItem.children.at(i);
        QAction *action = d->m_actionForId.value(dbusMenuItem.id);

        if (action) {
            // Update properties
            d->updateAction(action, dbusMenuItem.properties);
            if (menu && action->parent() != menu) {
                action->setParent(menu);
            }
        } else {
            // Create
            const int id = dbusMenuItem.id;
            action = d->createAction(id, dbusMenuItem.properties, menu);
            d->m_actionForId.insert(id, action);

            connect(action, &QObject::destroyed, this, [this, id]() {
                d->m_actionForId.remove(id);
            });

            connect(action, &QAction::triggered, this, [id, this]() {
                sendClickedEvent(id);
            });

            if (QMenu *menuAction = action->menu()) {
                connect(menuAction, &QMenu::aboutToShow, this, &DBusMenuImporter::slotMenuAboutToShow, Qt::UniqueConnection);
            }
        }

        // Ensure correct position.
        QAction *before = (i < currentActions.count()) ? currentActions.at(i) : nullptr;
        if (before != action) {
            // If action was already in menu, insertAction will move it.
            menu->insertAction(before, action);
            // Refresh the cached actions list after a move or insert
            currentActions = menu->actions();
        }
    }

    // 2. Remove actions no longer present
    for (QAction *action : std::as_const(actions)) {
        const int id = action->property(DBUSMENU_PROPERTY_ID).toInt();
        if (!newIds.contains(id)) {
            if (menu) {
                menu->removeAction(action);
            }
            if (QMenu *subMenu = action->menu()) {
                subMenu->deleteLater();
            }
            action->deleteLater();
            d->m_actionForId.remove(id);
        }
    }

    // 3. For the root menu, ensure every non-separator action has a submenu.
    //    Some apps (e.g. DBeaver/Eclipse) don't send "children-display: submenu"
    //    for certain top-level items, leaving them as flat actions. This makes
    //    them un-openable and invisible to search. Create empty submenus so they
    //    can be populated on demand via AboutToShow/GetLayout.
    if (parentId == 0) {
        for (QAction *action : menu->actions()) {
            if (!action->isSeparator() && !action->menu()) {
                const int id = action->property(DBUSMENU_PROPERTY_ID).toInt();
                QMenu *subMenu = createMenu(menu);
                subMenu->menuAction()->setProperty(DBUSMENU_PROPERTY_ID, id);
                action->setMenu(subMenu);
                d->m_actionForId.insert(id, action);
                connect(subMenu, &QMenu::aboutToShow, this, &DBusMenuImporter::slotMenuAboutToShow, Qt::UniqueConnection);
            }
        }
    }

    connect(menu, &QMenu::aboutToHide, this, &DBusMenuImporter::slotMenuAboutToHide, Qt::UniqueConnection);
    menu->setUpdatesEnabled(true);
    Q_EMIT menuUpdated(menu);
}

void DBusMenuImporter::sendClickedEvent(int id)
{
    d->sendEvent(id, QStringLiteral("clicked"));
}

void DBusMenuImporter::updateMenu()
{
    updateMenu(DBusMenuImporter::menu());
}

void DBusMenuImporter::updateMenu(QMenu *menu)
{
    Q_ASSERT(menu);

    QAction *action = menu->menuAction();
    if (!action) {
        return;
    }

    const int id = action->property(DBUSMENU_PROPERTY_ID).toInt();

    if (d->m_idsRefreshedByAboutToShow.contains(id)) {
        return; // Update already in progress, ignore re-entrant call.
    }
    d->m_idsRefreshedByAboutToShow << id;

    const auto call = d->m_interface->AboutToShow(id);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    if (!watcher) {
        d->m_idsRefreshedByAboutToShow.remove(id);
        return;
    }
    watcher->setProperty(DBUSMENU_PROPERTY_ID, id);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &DBusMenuImporter::slotAboutToShowDBusCallFinished);

    // Firefox deliberately ignores "aboutToShow" whereas Qt ignores" opened", so we'll just send both all the time...
    d->sendEvent(id, QStringLiteral("opened"));
}

void DBusMenuImporter::slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher *watcher)
{
    const int id = watcher->property(DBUSMENU_PROPERTY_ID).toInt();
    watcher->deleteLater();

    QMenu *menu = d->menuForId(id);
    if (!menu) {
        qCWarning(DBUSMENUQT) << "AboutToShow: no menu found for id" << id << "- layout will not be fetched";
        d->m_idsRefreshedByAboutToShow.remove(id);
        return;
    }

    const QDBusPendingReply<bool> reply = *watcher;
    if (reply.isError()) {
        d->m_idsRefreshedByAboutToShow.remove(id);
        qCWarning(DBUSMENUQT) << "Call to AboutToShow() failed:" << reply.error().message();
        Q_EMIT menuUpdated(menu);
        return;
    }
    // We used to only refresh if needRefresh was true.
    // However, some servers are buggy and don't signal correctly, or signals are lost.
    // Since this is called JIT before showing a menu, always refreshing is safer
    d->refresh(id);
}

void DBusMenuImporter::slotMenuAboutToHide()
{
    QMenu *menu = qobject_cast<QMenu *>(sender());
    Q_ASSERT(menu);

    QAction *action = menu->menuAction();
    Q_ASSERT(action);

    const int id = action->property(DBUSMENU_PROPERTY_ID).toInt();
    d->sendEvent(id, QStringLiteral("closed"));
}

void DBusMenuImporter::slotMenuAboutToShow()
{
    QMenu *menu = qobject_cast<QMenu *>(sender());
    Q_ASSERT(menu);

    updateMenu(menu);
}

void DBusMenuImporter::slotActionHovered(QAction *action)
{
    if (action && action->menu() && action->menu()->actions().isEmpty()) {
        updateMenu(action->menu());
    }
}

QMenu *DBusMenuImporter::createMenu(QWidget *parent)
{
    return new QMenu(parent);
}

QIcon DBusMenuImporter::iconForName(const QString &name)
{
    return QIcon::fromTheme(name);
}

#include "moc_dbusmenuimporter.cpp"
