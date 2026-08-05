/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2019 Zain Ahmad <zain.x.ahmad@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "appmenu/overflowbutton.h"
#include "breezedecoration.h"

#include <KDecoration3/DecoratedWindow>

#include <QPainter>

namespace Breeze
{

AppMenuOverflowButton::AppMenuOverflowButton(Decoration *decoration, const int buttonIndex, QObject *parent)
    : AppMenuButton(decoration, buttonIndex, parent)
{
    auto *decoratedClient = decoration->window();
    setVisible(decoratedClient->hasApplicationMenu());
}

AppMenuOverflowButton::~AppMenuOverflowButton()
{
}

void AppMenuOverflowButton::paintIcon(QPainter *painter, const QRectF &iconRect, const qreal)
{
    Q_UNUSED(iconRect)
    setPenWidth(painter, 1.5);

    const QPointF center = iconRect.center();
    const qreal spacing = 4.0;
    for (int i = -1; i <= 1; ++i) {
        const qreal y = center.y() + i * spacing;
        painter->drawLine(QPointF(center.x() - 5.5, y), QPointF(center.x() + 5.5, y));
    }
}

} // namespace Breeze
