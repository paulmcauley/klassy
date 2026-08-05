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

#include "appmenu/navigablemenu.h"

#include <KWindowEffects>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QStyle>

namespace Breeze
{

NavigableMenu::NavigableMenu(QWidget *parent, QStyle *style)
    : QMenu(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setStyle(style);
}

void NavigableMenu::keyPressEvent(QKeyEvent *event)
{
    // First, let the base class handle standard key presses (like up, down, enter).
    QMenu::keyPressEvent(event);

    // If the event was not accepted by the base class, check for our custom keys.
    if (!event->isAccepted()) {
        bool isRtl = QGuiApplication::layoutDirection() == Qt::RightToLeft;
        const Qt::Key leftKey = isRtl ? Qt::Key_Right : Qt::Key_Left;
        const Qt::Key rightKey = isRtl ? Qt::Key_Left : Qt::Key_Right;

        const bool isLeft = (event->key() == leftKey);
        const bool isRight = (event->key() == rightKey);

        if ((isLeft || isRight) && event->modifiers() == Qt::NoModifier) {
            // Do not navigate away if a submenu is open
            if (activeAction() && activeAction()->menu() && activeAction()->menu()->isVisible()) {
                return;
            }

            if (isLeft) {
                Q_EMIT hitLeft();
            } else {
                Q_EMIT hitRight();
            }
            event->accept();
        }
    }
}

void NavigableMenu::showEvent(QShowEvent *event)
{
    QMenu::showEvent(event);
    if (windowHandle()) {
        KWindowEffects::enableBlurBehind(windowHandle());
    }
}

} // namespace Breeze
