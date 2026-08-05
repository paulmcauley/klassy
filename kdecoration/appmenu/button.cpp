/*
 * Copyright (C) 2020 Chris Holland <zrenfire@gmail.com>
 * Copyright (C) 2026 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 * Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2014 by Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "appmenu/button.h"
#include "appmenu/buttongroup.h"
#include "breezedecoration.h"

#include <KDecoration3/DecoratedWindow>

#include <QApplication>
#include <QPainter>

namespace Breeze
{

AppMenuButton::AppMenuButton(Decoration *decoration, const int buttonIndex, QObject *parent)
    : KDecoration3::DecorationButton(KDecoration3::DecorationButtonType::Custom, decoration, parent)
    , m_buttonIndex(buttonIndex)
{
    setCheckable(true);

    connect(this, &AppMenuButton::clicked, this, &AppMenuButton::trigger);

    const auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent);
    if (buttonGroup) {
        setOpacity(buttonGroup->opacity());
    }
}

int AppMenuButton::buttonIndex() const
{
    return m_buttonIndex;
}

void AppMenuButton::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    const auto *deco = qobject_cast<Decoration *>(decoration());
    if (!deco) {
        return;
    }

    painter->save();

    painter->translate(geometry().topLeft());

    const QRectF iconRect(QPointF(0, 0), geometry().size());

    // Paint background
    const QColor bgColor = deco->titleBarColor();

    const auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent());
    if (buttonGroup && buttonGroup->isMenuOpen() && buttonGroup->currentIndex() != m_buttonIndex) {
        // Dimmed state
    } else if (buttonGroup && buttonGroup->isMenuOpen() && buttonGroup->currentIndex() == m_buttonIndex) {
        // Active menu button - paint highlight background
        QColor highlight = deco->window()->isActive() ? qApp->palette().color(QPalette::Highlight) : bgColor;
        highlight.setAlpha(bgColor.alpha());
        painter->fillRect(iconRect, highlight);
    } else if (isChecked()) {
        painter->fillRect(iconRect, bgColor);
    }

    paintIcon(painter, iconRect, 1.0);

    painter->restore();
}

void AppMenuButton::setPenWidth(QPainter *painter, qreal width)
{
    const qreal dpr = painter->deviceTransform().m11();
    QPen pen = painter->pen();
    pen.setWidthF(width * dpr);
    painter->setPen(pen);
}

qreal AppMenuButton::transitionValue() const
{
    return 1.0;
}

void AppMenuButton::setHeight(qreal buttonHeight)
{
    const QSizeF size(buttonHeight * 1.1, buttonHeight);
    setGeometry(QRectF(geometry().topLeft(), size));
}

void AppMenuButton::trigger()
{
    auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent());
    if (buttonGroup) {
        buttonGroup->trigger(m_buttonIndex);
    }
}

} // namespace Breeze
