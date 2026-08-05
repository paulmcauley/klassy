/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

// Qt
#include <QMetaType>
#include <QStringList>

class QKeySequence;

class DBusMenuShortcut : public QList<QStringList>
{
public:
    QKeySequence toKeySequence() const;
    static DBusMenuShortcut fromKeySequence(const QKeySequence &);
};
