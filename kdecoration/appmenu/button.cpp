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
#include <KWindowSystem>

#include <QApplication>
#include <QPainter>

namespace Breeze
{

AppMenuButton::AppMenuButton(DecorationButtonType type, Decoration *decoration, const int buttonIndex, QObject *parent)
    : KDecoration3::DecorationButton(KDecoration3::DecorationButtonType::Custom, decoration, parent)
    , m_d(qobject_cast<Decoration *>(decoration))
    , m_buttonIndex(buttonIndex)
    , m_type(type)
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

    if (!m_d) {
        return;
    }

    auto c = m_d->window();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    // Nothing to paint when fully transparent
    if (qFuzzyIsNull(m_opacity)) {
        painter->restore();
        return;
    }
    painter->setOpacity(m_opacity);
    painter->translate(geometry().topLeft());

    setDevicePixelRatio(painter);
    m_buttonPalette = m_d->decorationColors()->buttonPalette(m_type);

    const QRectF iconRect(QPointF(0, 0), geometry().size());

    // Calculate device offset for icon drawing
    QPointF deviceOffsetDecorationTopLeftToIconTopLeft;
    QPointF topLeftButtonDeviceGeometry = painter->deviceTransform().map(geometry().topLeft());
    QPointF decorationTopLeftDeviceGeometry = painter->deviceTransform().map(QRectF(m_d->rect()).topLeft());
    deviceOffsetDecorationTopLeftToIconTopLeft = topLeftButtonDeviceGeometry - decorationTopLeftDeviceGeometry;

    // Paint background
    const QColor bgColor = m_d->titleBarColor();

    const auto *buttonGroup = qobject_cast<AppMenuButtonGroup *>(parent());
    if (buttonGroup && buttonGroup->isMenuOpen() && buttonGroup->currentIndex() != m_buttonIndex) {
        // Dimmed state
    } else if (buttonGroup && buttonGroup->isMenuOpen() && buttonGroup->currentIndex() == m_buttonIndex) {
        // Active menu button - paint highlight background
        QColor highlight = c->isActive() ? qApp->palette().color(QPalette::Highlight) : bgColor;
        const int titleBarOpacity = m_d->internalSettings()->activeTitleBarOpacity();
        if (titleBarOpacity < 100) {
            painter->setCompositionMode(QPainter::CompositionMode_Source);
            highlight.setAlphaF(titleBarOpacity / 100.0f);
        }
        painter->fillRect(iconRect, highlight);
    } else if (isChecked()) {
        painter->fillRect(iconRect, bgColor);
    }

    drawIcon(painter, deviceOffsetDecorationTopLeftToIconTopLeft);

    painter->restore();
}

void AppMenuButton::setDevicePixelRatio(QPainter *painter)
{
    // on X11 Kwin just returns 1.0 for the DPR instead of the correct value, so use the scaling setting directly
    if (KWindowSystem::isPlatformX11())
        m_devicePixelRatio = m_d->systemScaleFactorX11();
    else
        m_devicePixelRatio = painter->device()->devicePixelRatioF();
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
