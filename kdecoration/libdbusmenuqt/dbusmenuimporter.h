/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

// Qt
#include <QObject>

// STL
#include <memory>

class QAction;
class QDBusPendingCallWatcher;
class QIcon;
class QMenu;

class DBusMenuImporterPrivate;

/**
 * A DBusMenuImporter instance can recreate a menu serialized over DBus by
 * DBusMenuExporter
 */
class DBusMenuImporter : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a DBusMenuImporter listening over DBus on service, path
     */
    DBusMenuImporter(const QString &service, const QString &path, QObject *parent = nullptr);

    ~DBusMenuImporter() override;

    QAction *actionForId(int id) const;

    /**
     * The menu created from listening to the DBusMenuExporter over DBus
     */
    QMenu *menu() const;

public Q_SLOTS:
    /**
     * Load the menu
     *
     * Will Q_EMIT menuUpdated() when complete.
     * This should be done before showing a menu
     */
    void updateMenu();

    void updateMenu(QMenu *menu);

Q_SIGNALS:
    /**
     * Emitted after a call to updateMenu().
     * @see updateMenu()
     */
    void menuUpdated(QMenu *);

    /**
     * Emitted when the exporter was asked to activate an action
     */
    void actionActivationRequested(QAction *);

protected:
    /**
     * Must create a menu, may be customized to fit host appearance.
     * Default implementation creates a simple QMenu.
     */
    virtual QMenu *createMenu(QWidget *parent);

    /**
     * Must convert a name into an icon.
     * Default implementation returns a null icon.
     */
    virtual QIcon iconForName(const QString &);

private Q_SLOTS:
    void sendClickedEvent(int);
    void slotActionHovered(QAction *);
    void slotMenuAboutToShow();
    void slotMenuAboutToHide();
    void slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher *);
    void slotItemActivationRequested(int id, uint timestamp);
    void processPendingLayoutUpdates();
    void slotLayoutUpdated(uint revision, int parentId);
    void slotGetLayoutFinished(QDBusPendingCallWatcher *);

private:
    Q_DISABLE_COPY(DBusMenuImporter)
    std::unique_ptr<DBusMenuImporterPrivate> const d;
    friend class DBusMenuImporterPrivate;
};
