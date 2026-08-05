/* This file is part of the dbusmenu-qt library
    SPDX-FileCopyrightText: 2009 Canonical
    SPDX-FileContributor: Aurelien Gateau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "dbusmenushortcut_p.h"

// Qt
#include <QKeySequence>
#include <QStringTokenizer>

// STL
#include <utility>

static const int QT_COLUMN = 0;
static const int DM_COLUMN = 1;

struct Row {
    const char *zero;
    const char *one;
};

static const Row table[] = {{"Meta", "Super"},
                            {"Ctrl", "Control"},
                            // Special cases for compatibility with libdbusmenu-glib which uses
                            // "plus" for "+" and "minus" for "-".
                            // cf https://bugs.launchpad.net/libdbusmenu-qt/+bug/712565
                            {"+", "plus"},
                            {"-", "minus"},
                            {nullptr, nullptr}};

static inline QLatin1StringView translate(QStringView token, int srcCol, int dstCol)
{
    for (const Row *ptr = table; ptr->zero != nullptr; ++ptr) {
        const char *from = (srcCol == QT_COLUMN ? ptr->zero : ptr->one);
        if (token == QLatin1StringView(from)) {
            return QLatin1StringView(dstCol == QT_COLUMN ? ptr->zero : ptr->one);
        }
    }
    return {};
}

DBusMenuShortcut DBusMenuShortcut::fromKeySequence(const QKeySequence &sequence)
{
    const QString string = sequence.toString();
    DBusMenuShortcut shortcut;
    for (auto token : QStringTokenizer{string, QLatin1StringView(", "), Qt::SkipEmptyParts}) {
        if (token == QLatin1StringView("+")) {
            shortcut.append({QLatin1String("plus")});
            continue;
        }

        // Hack: Qt::CTRL | Qt::Key_Plus is turned into the string "Ctrl++",
        // but we don't want the call to token.split() to consider the
        // second '+' as a separator so we handle it by checking if the token
        // ends with "++".
        const bool endsWithPlusPlus = token.endsWith(QLatin1StringView("++"));
        const QStringView subToken = endsWithPlusPlus ? token.chopped(2) : token;

        QStringList keyTokens;
        keyTokens.reserve(4);
        for (auto kt : QStringTokenizer{subToken, QLatin1Char('+'), Qt::SkipEmptyParts}) {
            if (const auto t = translate(kt, QT_COLUMN, DM_COLUMN); !t.isEmpty()) {
                keyTokens.append(t);
            } else {
                keyTokens.append(kt.toString());
            }
        }

        if (endsWithPlusPlus) {
            keyTokens.append(QLatin1String("plus"));
        }

        shortcut.append(std::move(keyTokens));
    }
    return shortcut;
}

QKeySequence DBusMenuShortcut::toKeySequence() const
{
    QString res;
    // Heuristic: estimate size to minimize reallocations.
    // Each shortcut part is at least a few chars, plus separators.
    res.reserve(size() * 16);

    for (const QStringList &keyTokens : std::as_const(*this)) {
        if (keyTokens.isEmpty()) {
            continue;
        }
        if (!res.isEmpty()) {
            res += QLatin1StringView(", ");
        }
        bool first = true;
        for (const QString &token : keyTokens) {
            if (!first) {
                res += QLatin1Char('+');
            }
            first = false;
            if (const auto t = translate(token, DM_COLUMN, QT_COLUMN); !t.isEmpty()) {
                res += t;
            } else {
                res += token;
            }
        }
    }
    return QKeySequence::fromString(res);
}
