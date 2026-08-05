/*
 * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail.com>
 * Based on https://invent.kde.org/plasma/breeze/-/merge_requests/529/diffs
 *          Copyright (C) 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
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
 */

#pragma once

#include <QMenu>

namespace Breeze
{

class NavigableMenu : public QMenu
{
    Q_OBJECT

public:
    explicit NavigableMenu(QWidget *parent, QStyle *style);
    ~NavigableMenu() override = default;

Q_SIGNALS:
    void hitLeft();
    void hitRight();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

} // namespace Breeze
