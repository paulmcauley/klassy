/*
 * Copyright (C) 2025 Guido Iodice <guido[dot]iodice[at]gmail[dot]com>
 *
 * This program is free software: you can redistribute it and/or modify
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

// own
#include "appmenu/iconbutton.h"
#include "breeze.h"
#include "breezedecoration.h"
#include "renderdecorationbuttonicon.h"
#include "systemicontheme.h"

// Qt
#include <QPainter>
#include <QtMath>

namespace Klassy
{

AppMenuIconButton::AppMenuIconButton(const DecorationButtonType type, Decoration *decoration, const int buttonIndex, AppMenuButtonGroup *parent)
    : AppMenuButton(type, decoration, buttonIndex, parent)
{
    reconfigure();
    connect(this, &AppMenuButton::buttonHeightChanged, this, &AppMenuIconButton::updateGeometry);
    connect(this, &AppMenuButton::verticalBackgroundOffsetChanged, this, &AppMenuIconButton::updateGeometry);
    reconfigure();
    updateGeometry();
}

AppMenuIconButton::~AppMenuIconButton() = default;

void AppMenuIconButton::updateGeometry()
{
    if (!m_d) {
        return;
    }

    const qreal paddedSize = m_d->smallButtonPaddedSize();
    setGeometry(QRectF(geometry().topLeft(), QSizeF(paddedSize, buttonHeight())));
    setSmallButtonPaddedWidth(paddedSize);
    setIconWidth(m_d->iconSize());
    setBackgroundVisibleSize(QSizeF(paddedSize, buttonHeight() - verticalBackgroundOffset() * 2));
}

void AppMenuIconButton::drawContent(QPainter *painter, QPointF offsetDecorationTopLeftToContentTopLeft) const
{
    if (!m_d) {
        return;
    };

    if (iconWidth() <= 0 || smallButtonPaddedWidth() <= 0) {
        return;
    }

    // Center icon in button
    painter->translate(this->geometry().topLeft() - offsetDecorationTopLeftToContentTopLeft + iconOffset());
    QPointF deviceOffsetDecorationTopLeftToIconTopLeft = offsetDecorationTopLeftToContentTopLeft * painter->device()->devicePixelRatioF();

    // Setup pen for icon drawing
    QPen pen(foregroundColor());
    pen.setWidthF(PenWidth::Symbol);
    pen.setCosmetic(true);
    painter->setPen(pen);

    bool renderSystemIcon =
        m_d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme && isSystemIconAvailable();

    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (renderSystemIcon) {
        auto c = m_d->window();
        QString systemIconName;
        systemIconName = isChecked() ? m_systemIconCheckedName : m_systemIconName;
        SystemIconTheme iconRenderer(painter,
                                     iconWidth(),
                                     systemIconName,
                                     m_d->internalSettings(),
                                     m_d->internalSettings()->forceColorizeSystemIcons() ? QPalette() : c->palette());
        iconRenderer.renderIcon();
    } else {
        auto [iconRenderer, localRenderingWidth] = RenderDecorationButtonIcon::factory(m_d->internalSettings(), painter);

        iconRenderer->setBoldButtonIcons(this->shouldDrawBoldButtonIcons());
        iconRenderer->setSystemScale(m_systemScale);
        iconRenderer->setDeviceOffsetFromZeroReference(deviceOffsetDecorationTopLeftToIconTopLeft);

        // at loDPI backgrounds are even, therefore need an even icon in such circumstances for correct centring
        bool forceEvenSquares = (m_systemScale <= 1.001
                                 && (m_d->buttonBackgroundType() == ButtonBackgroundType::Small
                                     || m_d->internalSettings()->iconSize() < InternalSettings::EnumIconSize::IconLargeMedium));

        iconRenderer->setForceEvenSquares(forceEvenSquares);

        qreal scaleFactor = iconWidth() / localRenderingWidth;
        /*
        scale painter so that all further rendering is preformed inside QRect( 0, 0, localRenderingWidth, localRenderingWidth )
        */
        painter->scale(scaleFactor, scaleFactor);

        iconRenderer->renderIcon(m_type, isChecked());
    }
}

void AppMenuIconButton::reconfigure()
{
    AppMenuButton::reconfigure();
    // set m_systemIconName and m_systemIconCheckedName if a system icon theme is set
    if (m_d->internalSettings()->buttonIconStyle() == InternalSettings::EnumButtonIconStyle::StyleSystemIconTheme) {
        SystemIconTheme::systemIconNames(m_type, m_systemIconName, m_systemIconCheckedName);
    }
}

bool AppMenuIconButton::shouldDrawBoldButtonIcons() const
{
    if (!m_d)
        return false;

    switch (m_d->internalSettings()->boldButtonIcons()) {
    case InternalSettings::EnumBoldButtonIcons::BoldIconsActiveHiDpi:
        return m_d->window()->isActive() && m_systemScale > 1.2;
    case InternalSettings::EnumBoldButtonIcons::BoldIconsActive:
        return m_d->window()->isActive();
    case InternalSettings::EnumBoldButtonIcons::BoldIconsHiDpiOnly:
        // If HiDPI system scaling use bold icons
        return m_systemScale > 1.2;
    case InternalSettings::EnumBoldButtonIcons::BoldIconsBold:
        return true;
    case InternalSettings::EnumBoldButtonIcons::BoldIconsFine:
    default:
        return false;
    }
}

bool AppMenuIconButton::isSystemIconAvailable() const
{
    if (isChecked()) {
        if (m_systemIconCheckedName.isEmpty())
            return false;
        else
            return true;
    } else {
        if (m_systemIconName.isEmpty())
            return false;
        else
            return true;
    }
}

} // namespace Klassy
