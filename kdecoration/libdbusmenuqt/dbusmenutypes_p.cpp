/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenutypes_p.h"

// Local
#include "dbusmenushortcut_p.h"

// Qt
#include <QDBusArgument>
#include <QDBusMetaType>

//// DBusMenuItem
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItem &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItem &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.endStructure();
    return argument;
}

//// DBusMenuItemKeys
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItemKeys &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItemKeys &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.endStructure();
    return argument;
}

//// DBusMenuLayoutItem
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuLayoutItem &obj)
{
    argument.beginStructure();
    argument << obj.id << obj.properties;
    argument.beginArray(qMetaTypeId<QDBusVariant>());
    for (const DBusMenuLayoutItem &child : obj.children) {
        argument << QDBusVariant(QVariant::fromValue<DBusMenuLayoutItem>(child));
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuLayoutItem &obj)
{
    argument.beginStructure();
    argument >> obj.id >> obj.properties;
    argument.beginArray();
    while (!argument.atEnd()) {
        QDBusVariant dbusVariant;
        argument >> dbusVariant;
        QDBusArgument childArgument = dbusVariant.variant().value<QDBusArgument>();

        DBusMenuLayoutItem child;
        childArgument >> child;
        obj.children.append(std::move(child));
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

//// DBusMenuShortcut
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuShortcut &obj)
{
    argument.beginArray(qMetaTypeId<QStringList>());
    for (const QStringList &shortcut : obj) {
        argument << shortcut;
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuShortcut &obj)
{
    argument.beginArray();
    obj.clear();
    while (!argument.atEnd()) {
        QStringList item;
        argument >> item;
        obj.push_back(std::move(item));
    }
    argument.endArray();
    return argument;
}

void DBusMenuTypes_register()
{
    static bool registered = false;
    if (registered) {
        return;
    }
    qDBusRegisterMetaType<DBusMenuItem>();
    qDBusRegisterMetaType<DBusMenuItemList>();
    qDBusRegisterMetaType<DBusMenuItemKeys>();
    qDBusRegisterMetaType<DBusMenuItemKeysList>();
    qDBusRegisterMetaType<DBusMenuLayoutItem>();
    qDBusRegisterMetaType<DBusMenuLayoutItemList>();
    qDBusRegisterMetaType<DBusMenuShortcut>();
    registered = true;
}
